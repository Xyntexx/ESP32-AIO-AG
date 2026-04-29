#include "keya_motor.h"

#if KEYA_MOTOR

#include "../../autosteer/motor.h"
#include "../can/can_manager.h"
#include "../../utils/log.h"

namespace hw {

// Keya CAN protocol constants (see memory/keya_teensy_reference.md)
static constexpr uint32_t KEYA_TX_ID        = 0x06000001; // outgoing commands
static constexpr uint32_t KEYA_HEARTBEAT_ID = 0x07000001; // incoming heartbeat

// Heartbeat staleness watchdog. Default cadence is 20ms, so 200ms is 10
// missed frames. Anything older than this marks the motor unhealthy.
static constexpr uint32_t KEYA_STALE_MS = 200;

// Speed mapping: autosteer pwm magnitude is 0..255, sign comes from the
// `reversed` flag. Keya wants signed int16 speed in [-995..998].
static constexpr int16_t KEYA_SPEED_MAX = 998;
static constexpr int16_t KEYA_SPEED_MIN = -995;

bool KeyaMotor::initialized = false;
uint8_t KeyaMotor::currentPwm = 0;

static volatile bool     healthyFlag    = false;
static volatile uint32_t lastSeenMs     = 0;
static volatile uint16_t lastErrorCode  = 0;
static volatile uint16_t lastCurrentMA  = 0;

static void buildBaseFrame(twai_message_t& msg) {
    memset(&msg, 0, sizeof(msg));
    msg.identifier = KEYA_TX_ID;
    msg.extd = 1;
    msg.data_length_code = 8;
}

static void sendEnable() {
    twai_message_t m;
    buildBaseFrame(m);
    m.data[0] = 0x23;
    m.data[1] = 0x0D;
    m.data[2] = 0x20;
    m.data[3] = 0x01;
    hw::can::send(m);
}

static void sendDisable() {
    twai_message_t m;
    buildBaseFrame(m);
    m.data[0] = 0x23;
    m.data[1] = 0x0C;
    m.data[2] = 0x20;
    m.data[3] = 0x01;
    hw::can::send(m);
}

static void sendSpeed(int16_t signedSpeed) {
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
    info("KeyaMotor initialized");
    return true;
}

void KeyaMotor::drive(uint8_t pwm, bool reversed) {
    if (!initialized) return;

    if (pwm == 0) {
        sendDisable();
        currentPwm = 0;
        return;
    }

    if (!isHealthy()) {
        // Keya heartbeat missing or in fault. Don't drive blind - the user-
        // facing engage gate (autosteer.cpp) refuses engagement on unhealthy,
        // but a stale heartbeat between checks would still get here. Log
        // throttled via the `error counter above warn` path in can_manager.
        sendDisable();
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

    twai_message_t msg;
    while (hw::can::receive(msg, pdMS_TO_TICKS(50))) {
        if (msg.identifier != KEYA_HEARTBEAT_ID) continue;
        if (msg.data_length_code < 8) continue;

        // Heartbeat layout (from keya_teensy_reference.md):
        //   [0..1] cumulative angle (signed)
        //   [2..3] motor speed (signed)
        //   [4..5] motor current (with symbol)
        //   [6..7] error code
        uint16_t errCode = ((uint16_t)msg.data[6] << 8) | msg.data[7];
        // Current scaling matches the Teensy reference: signed byte 4 acts as
        // sign, byte 5 magnitude * 20 yields milliamps.
        uint16_t curMA;
        if (msg.data[4] == 0xFF) {
            curMA = (256 - msg.data[5]) * 20;
        } else {
            curMA = msg.data[5] * 20;
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
        lastSeenMs    = millis();
        healthyFlag   = (errCode == 0);
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

} // namespace hw

#endif // KEYA_MOTOR
