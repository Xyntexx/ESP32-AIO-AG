#include "safety.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "config/defines.h"
#include "autosteer.h"
#include "buttons.h"
#include "motor.h"
#include "udp_io.h"
#include "utils/log.h"

#if KEYA_MOTOR
#include "hardware/motor/keya_motor.h"
#endif

namespace safety {

namespace {

volatile uint32_t s_autosteerTicks  = 0;
volatile uint32_t s_lastTickMs      = 0;
volatile uint32_t s_stops           = 0;

StaticTask_t       s_taskCb;
StackType_t        s_taskStack[2048];

constexpr uint32_t TICK_MS                = SAFETY_TICK_MS;
constexpr uint32_t AUTOSTEER_STALE_MS     = SAFETY_AUTOSTEER_STALE_MS;

enum class Reason : uint8_t {
    None = 0,
    Disengaged,
    GuidanceStale,
    AutosteerStale,
    KeyaUnhealthy,
};

const char* reasonStr(Reason r) {
    switch (r) {
        case Reason::None:           return "none";
        case Reason::Disengaged:     return "disengaged";
        case Reason::GuidanceStale:  return "guidance-stale";
        case Reason::AutosteerStale: return "autosteer-stale";
        case Reason::KeyaUnhealthy:  return "keya-unhealthy";
    }
    return "?";
}

// Check the safety conditions and return the first one that requires a
// stop. Order matters only for log clarity. We don't try to enumerate
// every reason - first match wins.
Reason checkConditions(uint32_t now) {
    // 1) Composite engage state - if either physical button or software
    //    switch is off, autosteer should be stopped. This is what
    //    autosteer::handler() already enforces; the safety task just
    //    re-asserts it independent of whether handler() got CPU.
    if (!autosteer::isEngaged()) {
        return Reason::Disengaged;
    }

    // 2) AOG guidance stale - autosteer setpoint is older than
    //    WATCHDOG_TIMEOUT (currently 500 ms). On the bench with no AOG,
    //    this is the dominant trigger. In the field this would fire
    //    if the network drops (the original v0.0.9 failure mode).
    if (!guidancePacketValid()) {
        return Reason::GuidanceStale;
    }

    // 3) Autosteer task heartbeat - if the 1 kHz handler hasn't run
    //    within AUTOSTEER_STALE_MS, something is starving it (network
    //    wedge, mutex pile-up). Stop the motor regardless.
    uint32_t lastTick = s_lastTickMs;
    if (lastTick != 0 && (now - lastTick) > AUTOSTEER_STALE_MS) {
        return Reason::AutosteerStale;
    }

#if KEYA_MOTOR
    // 4) Keya heartbeat lost - the existing autosteer handler refuses
    //    to engage if unhealthy, but if it transitions to unhealthy
    //    mid-engage we want to drop the motor immediately.
    if (!hw::KeyaMotor::isHealthy()) {
        return Reason::KeyaUnhealthy;
    }
#endif

    return Reason::None;
}

[[noreturn]] void task(void*) {
    Reason lastReason = Reason::None;
    for (;;) {
        uint32_t now = millis();
        Reason r = checkConditions(now);

        if (r != Reason::None) {
            motor::stopMotor();
            if (r != lastReason) {
                // Only log on transition - we'd rather flood than miss
                // it, but actual tractor-field runs don't need a log
                // line every 20 ms once the tractor is parked.
                warningf("safety: stop reason=%s", reasonStr(r));
                s_stops++;
            }
        }
        lastReason = r;

        vTaskDelay(pdMS_TO_TICKS(TICK_MS));
    }
}

} // namespace

bool init() {
    TaskHandle_t h = xTaskCreateStatic(
        task,
        "safety",
        sizeof(s_taskStack) / sizeof(StackType_t),
        nullptr,
        SAFETY_TASK_PRIORITY,
        s_taskStack,
        &s_taskCb);
    if (!h) {
        error("safety: task create failed");
        return false;
    }
    infof("safety: started (tick=%lu ms, prio=%d, autosteer-stale=%lu ms)",
          (unsigned long)TICK_MS,
          SAFETY_TASK_PRIORITY,
          (unsigned long)AUTOSTEER_STALE_MS);
    return true;
}

void noteAutosteerTick() {
    s_autosteerTicks++;
    s_lastTickMs = millis();
}

uint32_t stopCount() {
    return s_stops;
}

} // namespace safety
