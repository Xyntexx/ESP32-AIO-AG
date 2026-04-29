#include "keya_motor.h"

#if KEYA_MOTOR

#include "../../autosteer/motor.h"
#include "../can/can_manager.h"
#include "../../utils/log.h"

namespace hw {

// Keya CAN protocol constants (see memory/keya_teensy_reference.md)
static constexpr uint32_t KEYA_TX_ID         = 0x06000001; // outgoing commands
static constexpr uint32_t KEYA_HEARTBEAT_ID  = 0x07000001; // incoming heartbeat
static constexpr uint32_t KEYA_SDO_RESP_ID   = 0x05800001; // SDO-server response

// Heartbeat staleness watchdog. Default cadence is 20ms, so 200ms is 10
// missed frames. Anything older than this marks the motor unhealthy.
static constexpr uint32_t KEYA_STALE_MS = 200;

// Speed mapping: autosteer pwm magnitude is 0..255, sign comes from the
// `reversed` flag. Keya wants signed int16 speed in [-995..998].
static constexpr int16_t KEYA_SPEED_MAX = 998;
static constexpr int16_t KEYA_SPEED_MIN = -995;

// Heartbeat error-code bit decode (per KY173 datasheet, sec 4.5.2).
// errCode is built as (Data6 << 8) | Data7.
// Data6 bits: 6=CAN break, 5=232 break, 4=current sensing, 3=Hall failure,
//             2=temp protection, 0=working mode failure.
// Data7 bits: 7=control signal, 6=overcurrent, 4=undervoltage, 3=E2PROM,
//             2=hardware protection, 1=overvoltage, 0=Disable.
//
// Bits we treat as STATE rather than fault:
//   - Data7 bit0 (0x0001 "Disable"): the motor truthfully reports it is
//     disabled. We send disable at init and on stop - that's expected.
//   - Data6 bit6 (0x4000 "CAN break"): means the motor has not seen a CAN
//     command for ~1s. Cleared by our keep-alive disable frame, so it
//     should rarely persist.
// Anything else is a real fault and blocks engagement.
static constexpr uint16_t KEYA_NONFAULT_MASK = 0x4001;
static constexpr uint16_t KEYA_FAULT_MASK    = ~KEYA_NONFAULT_MASK;

// Keep-alive cadence. Keya watchdog fires after 1000ms of CAN silence;
// 200ms gives us a healthy margin.
static constexpr uint32_t KEYA_KEEPALIVE_MS = 200;

bool KeyaMotor::initialized = false;
uint8_t KeyaMotor::currentPwm = 0;

static volatile bool     healthyFlag    = false;
static volatile uint32_t lastSeenMs     = 0;
static volatile uint16_t lastErrorCode  = 0;
static volatile uint16_t lastCurrentMA  = 0;
static volatile uint16_t peakCurrentMA  = 0;

// Cumulative encoder position. The Keya heartbeat reports a uint16 angle
// counter that wraps at 0xFFFF (one count per degree, per manual sec 4.5.2).
// On the first heartbeat we snapshot the raw value; thereafter we compute a
// signed 16-bit delta from the previous reading - that delta wraps correctly
// for small per-tick motions (typically <<1 deg per 20ms heartbeat) - and
// accumulate into a 32-bit signed running total in degrees from boot zero.
static volatile bool     posRefValid    = false;
static volatile uint16_t lastRawAngle   = 0;
static volatile int32_t  cumulativeDeg  = 0;

static void buildBaseFrame(twai_message_t& msg) {
    memset(&msg, 0, sizeof(msg));
    msg.identifier = KEYA_TX_ID;
    msg.extd = 1;
    msg.data_length_code = 8;
}

static void sendEnable() {
#if KEYA_LISTEN_ONLY
    return;
#else
    twai_message_t m;
    buildBaseFrame(m);
    m.data[0] = 0x23;
    m.data[1] = 0x0D;
    m.data[2] = 0x20;
    m.data[3] = 0x01;
    hw::can::send(m);
#endif
}

static void sendDisable() {
#if KEYA_LISTEN_ONLY
    return;
#else
    twai_message_t m;
    buildBaseFrame(m);
    m.data[0] = 0x23;
    m.data[1] = 0x0C;
    m.data[2] = 0x20;
    m.data[3] = 0x01;
    hw::can::send(m);
#endif
}

static void sendSpeed(int16_t signedSpeed) {
#if KEYA_LISTEN_ONLY
    (void)signedSpeed;
    return;
#else
    twai_message_t m;
    buildBaseFrame(m);
    m.data[0] = 0x23;
    m.data[1] = 0x00;
    m.data[2] = 0x20;
    m.data[3] = 0x01;
    m.data[4] = (uint8_t)((signedSpeed >> 8) & 0xFF); // hi
    m.data[5] = (uint8_t)(signedSpeed & 0xFF);        // lo
    if (signedSpeed < 0) {
        m.data[6] = 0xFF;
        m.data[7] = 0xFF;
    } else {
        m.data[6] = 0x00;
        m.data[7] = 0x00;
    }
    hw::can::send(m);
#endif
}

bool KeyaMotor::init() {
    if (initialized) return true;

    if (!hw::can::init()) {
        error("KeyaMotor: CAN init failed");
        return false;
    }

    // Wait briefly for the bus to settle, then send a disable so the motor
    // doesn't act on whatever stale command may have been latched.
    delay(50);
    sendDisable();

    motor::MotorInterface interface;
    interface.drive  = drive;
    interface.stop   = stop;
    interface.getPwm = getPwm;
    motor::init(interface);

    initialized = true;
#if KEYA_LISTEN_ONLY
    info("KeyaMotor initialized (LISTEN-ONLY: no CAN tx will be issued)");
#else
    info("KeyaMotor initialized");
#endif
    return true;
}

void KeyaMotor::drive(uint8_t pwm, bool reversed) {
    if (!initialized) return;

    if (!isHealthy()) {
        // Keya heartbeat missing or in fault. Don't drive blind - the user-
        // facing engage gate (autosteer.cpp) refuses engagement on unhealthy,
        // but a stale heartbeat between checks would still get here.
        sendDisable();
        currentPwm = 0;
        return;
    }

    if (pwm == 0) {
        // Zero error from PID. Hold position rather than coast: keep the
        // motor enabled with speed=0 so its closed-loop speed control
        // actively resists external rotation. Disabling here would let
        // the wheel drift off the setpoint until the next non-zero error.
        sendSpeed(0);
        sendEnable();
        currentPwm = 0;
        return;
    }

    int16_t signedPwm  = reversed ? -(int16_t)pwm : (int16_t)pwm;
    int16_t keyaSpeed  = (int16_t)map(signedPwm, -255, 255, KEYA_SPEED_MIN, KEYA_SPEED_MAX);

    sendSpeed(keyaSpeed);
    sendEnable();
    currentPwm = pwm;
}

void KeyaMotor::stop() {
    if (!initialized) return;
    sendSpeed(0);
    sendDisable();
    currentPwm = 0;
}

uint8_t KeyaMotor::getPwm() {
    return currentPwm;
}

void KeyaMotor::handler() {
    if (!initialized) return;

    static bool everSeen      = false;
    static bool prevHealthy   = false;
    static uint16_t prevError = 0;
    static uint32_t lastKeepaliveMs = 0;

    // Keep-alive: the motor's own watchdog (datasheet sec 4.5.1) trips after
    // 1s of CAN silence, raising the "CAN break" bit in the heartbeat error
    // code. Send a disable frame periodically when nothing else is being
    // sent so the motor stays in a clean disabled-but-connected state.
    if (millis() - lastKeepaliveMs > KEYA_KEEPALIVE_MS) {
        if (currentPwm == 0) {
            sendDisable();
        }
        lastKeepaliveMs = millis();
    }

    twai_message_t msg;
    while (hw::can::receive(msg, pdMS_TO_TICKS(50))) {
#if KEYA_SNIFFER
        // Sniffer: log "interesting" non-routine frames so the user can
        // reverse-engineer proprietary commands by running another master
        // (Keya ServoCAN tool) on the same bus. Filter out:
        //   - Heartbeats (own steady-state, parsed below).
        //   - SDO write-ACKs to OUR own commands (scs=0x60 + idx in
        //     {0x2000:00 = speed-ack, 0x200C:00 = disable-ack,
        //                                 0x200D:00 = enable-ack}).
        // What's left: any frame from the tool, plus any motor response
        // we did not initiate.
        bool isHeartbeat = (msg.identifier == KEYA_HEARTBEAT_ID);
        bool isOurAck = (msg.identifier == KEYA_SDO_RESP_ID
                         && msg.data_length_code == 8
                         && msg.data[0] == 0x60
                         && msg.data[2] == 0x20
                         && msg.data[3] == 0x00
                         && (msg.data[1] == 0x00 || msg.data[1] == 0x0C
                                                  || msg.data[1] == 0x0D));
        if (!isHeartbeat && !isOurAck) {
            debugf("CAN rx id=0x%08X dlc=%u %02X %02X %02X %02X %02X %02X %02X %02X",
                   msg.identifier, (unsigned)msg.data_length_code,
                   msg.data[0], msg.data[1], msg.data[2], msg.data[3],
                   msg.data[4], msg.data[5], msg.data[6], msg.data[7]);
        }
#endif

        // SDO responses (read/write replies) arrive on a different COB-ID
        // than heartbeats. Decode and log them on the debug stream so any
        // SDO-read utility yields visible data.
        if (msg.identifier == KEYA_SDO_RESP_ID && msg.data_length_code >= 8) {
            uint8_t  scs = msg.data[0];
            uint16_t idx = (uint16_t)msg.data[1] | ((uint16_t)msg.data[2] << 8);
            uint8_t  sub = msg.data[3];
            uint32_t data = (uint32_t)msg.data[4]
                          | ((uint32_t)msg.data[5] << 8)
                          | ((uint32_t)msg.data[6] << 16)
                          | ((uint32_t)msg.data[7] << 24);
            if (scs == 0x80) {
                // Abort transfer - object/subindex doesn't exist or is denied.
                debugf("Keya SDO abort idx=0x%04X sub=0x%02X code=0x%08X",
                       idx, sub, data);
            } else if ((scs & 0xE0) == 0x40) {
                // Upload response (0x42=4-byte, 0x43..0x4F=expedited variants).
                infof("Keya SDO read idx=0x%04X sub=0x%02X scs=0x%02X data=0x%08X (%d)",
                      idx, sub, scs, data, (int)data);
            } else {
                // Quiet about routine write-ACKs (scs=0x60) for our own
                // commands - those are the speed/disable/enable ACKs at
                // 0x2000/0x200C/0x200D and just clutter the log.
                bool routineAck = (scs == 0x60 && sub == 0x00
                                   && (idx == 0x2000 || idx == 0x200C
                                                     || idx == 0x200D));
                if (!routineAck) {
                    debugf("Keya SDO frame idx=0x%04X sub=0x%02X scs=0x%02X data=0x%08X",
                           idx, sub, scs, data);
                }
            }
            continue;
        }

        if (msg.identifier != KEYA_HEARTBEAT_ID) continue;
        if (msg.data_length_code < 8) continue;

        // Heartbeat layout (from keya_teensy_reference.md):
        //   [0..1] cumulative angle (signed)
        //   [2..3] motor speed (signed)
        //   [4..5] motor current (with symbol)
        //   [6..7] error code
        uint16_t errCode = ((uint16_t)msg.data[6] << 8) | msg.data[7];
        // Heartbeat current per manual sec 4.5.2: bytes [4..5] are a signed
        // 16-bit value, byte 4 high, byte 5 low (big-endian). The MANUAL DOES
        // NOT DOCUMENT THE UNIT. Earlier code (inherited from the lansalot
        // Teensy fork) treated byte 4 as a sign byte and byte 5 magnitude * 20
        // mA, which only matches an unsigned 0..5 A range and misinterprets
        // anything larger. With the corrected parse below, "currentRaw" is
        // the magnitude in whatever units the controller emits; map to mA via
        // KEYA_CURRENT_RAW_PER_MA once empirically determined.
        int16_t signedCurrent = (int16_t)(((uint16_t)msg.data[4] << 8) | msg.data[5]);
        uint16_t currentRaw = (uint16_t)((signedCurrent < 0) ? -signedCurrent
                                                             : signedCurrent);
        uint16_t curMA = currentRaw * KEYA_CURRENT_RAW_PER_MA;

#if KEYA_LOG_CURRENT_EVERY > 0
        // Calibration log: print raw bytes + decoded values periodically so
        // we can verify the unit scaling against actual measured current.
        static uint16_t logCurrentCounter = 0;
        if (++logCurrentCounter >= KEYA_LOG_CURRENT_EVERY) {
            logCurrentCounter = 0;
            infof("Keya cur: bytes[4..5]=%02X %02X signed=%d raw=%u curMA=%u",
                  msg.data[4], msg.data[5], (int)signedCurrent,
                  (unsigned)currentRaw, (unsigned)curMA);
        }
#endif

        // Cumulative angle (Data0 high, Data1 low; 1 LSB = 1 deg, wraps at
        // 0xFFFF per manual sec 4.5.2). On the first frame, snapshot the
        // raw value as the zero reference; afterwards, compute the signed
        // 16-bit delta and accumulate. uint16 subtraction wraps correctly
        // for small per-tick motions.
        uint16_t rawAngle = ((uint16_t)msg.data[0] << 8) | msg.data[1];
        if (!posRefValid) {
            lastRawAngle = rawAngle;
            cumulativeDeg = 0;
            posRefValid = true;
        } else {
            int16_t delta = (int16_t)(rawAngle - lastRawAngle);
            cumulativeDeg += delta;
            lastRawAngle = rawAngle;
        }

        if (!everSeen) {
            infof("Keya: first heartbeat received (current=%umA, errCode=0x%04X)",
                  curMA, errCode);
            everSeen = true;
        }
        if (errCode != prevError) {
            if (errCode == 0) {
                info("Keya: error cleared");
            } else {
                errorf("Keya: error code 0x%04X", errCode);
            }
            prevError = errCode;
        }

        lastErrorCode = errCode;
        lastCurrentMA = curMA;
        if (curMA > peakCurrentMA) peakCurrentMA = curMA;
        lastSeenMs    = millis();
        // Mask out non-fault state bits (Disable, CAN break) - those are
        // expected operating states, not motor faults.
        healthyFlag   = ((errCode & KEYA_FAULT_MASK) == 0);
    }

    // Staleness check - mark unhealthy if no heartbeat for KEYA_STALE_MS.
    if (healthyFlag && (millis() - lastSeenMs) > KEYA_STALE_MS) {
        healthyFlag = false;
    }

    // Transition logging so the operator sees health flips on the debug stream.
    bool curHealthy = healthyFlag;
    if (curHealthy != prevHealthy) {
        if (curHealthy) {
            info("Keya: healthy");
        } else {
            error("Keya: unhealthy (heartbeat stale or fault)");
        }
        prevHealthy = curHealthy;
    }
}

bool KeyaMotor::isHealthy() {
    if (!initialized) return false;
    if (!healthyFlag) return false;
    if ((millis() - lastSeenMs) > KEYA_STALE_MS) return false;
    return true;
}

uint16_t KeyaMotor::getCurrentMA() {
    return lastCurrentMA;
}

uint16_t KeyaMotor::getPeakCurrentMA() {
    return peakCurrentMA;
}

void KeyaMotor::resetPeakCurrent() {
    peakCurrentMA = 0;
}

int32_t KeyaMotor::getCumulativeDegrees() {
    return cumulativeDeg;
}

bool KeyaMotor::hasPositionRef() {
    return posRefValid;
}

bool KeyaMotor::sdoRead(uint16_t index, uint8_t subindex) {
    twai_message_t m;
    buildBaseFrame(m);
    m.data[0] = 0x40;                       // SDO initiate upload (read)
    m.data[1] = (uint8_t)(index & 0xFF);    // index low (CANopen LE)
    m.data[2] = (uint8_t)(index >> 8);      // index high
    m.data[3] = subindex;
    // Bytes 4..7 are reserved/zero on an upload request.
    return hw::can::send(m);
}

void KeyaMotor::probeMaxCurrent() {
    info("Keya: probing OD for max-current parameter...");

    // Confidence check first: the manual documents 0x2100:01 as "motor current
    // query". If our SDO mechanism is correct we should see a response with
    // the live current value. If this one is silent too, the issue is
    // somewhere in our CAN plumbing rather than the OD addressing.
    sdoRead(0x2100, 0x01);                  // KNOWN: live motor current
    delay(40);
    sdoRead(0x210F, 0x01);                  // KNOWN: motor temperature
    delay(40);
    sdoRead(0x210D, 0x02);                  // KNOWN: power supply voltage
    delay(40);

    // CANopen DSP402 motor-profile candidates.
    sdoRead(0x6073, 0x00);                  // Max Current
    delay(40);
    sdoRead(0x6075, 0x00);                  // Motor Rated Current
    delay(40);
    sdoRead(0x6076, 0x00);                  // Motor Rated Torque
    delay(40);

    // Keya-specific guesses for configuration parameter 3 (Max current).
    sdoRead(0x2003, 0x00);
    delay(40);
    sdoRead(0x2003, 0x01);
    delay(40);
    sdoRead(0x2103, 0x03);
    delay(40);
    sdoRead(0x210C, 0x03);
    delay(40);
    sdoRead(0x2200, 0x03);
    delay(40);
    sdoRead(0x2F03, 0x00);
    delay(40);
}

} // namespace hw

#endif // KEYA_MOTOR
