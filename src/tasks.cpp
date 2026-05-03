#include "tasks.h"

#include "autosteer/autosteer.h"
#include "autosteer/buttons.h"
#include "autosteer/safety.h"
#include "gps/gps_module.h"
#include "gps/gps_heading.h"
#if KEYA_WAS
#include "hardware/was/keya_was.h"
#else
#include "hardware/was/ads1115_was.h"
#endif
#include "hardware/imu/bno08x_imu.h"
#if KEYA_MOTOR
#include "hardware/motor/keya_motor.h"
#endif
#if SIMULATOR
#include "sim/bicycle_sim.h"
#endif
#if TEST_MODE
#include "test/hw_test.h"
#endif
#include "utils/log.h"

#if !KEYA_WAS
[[noreturn]] void was_task(void *pv_parameters) {
    for (;;) {
        hw::ADS1115WAS::handler();
        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz update rate
    }
}
#endif
// Under KEYA_WAS the WAS reading is pulled on demand from the Keya
// heartbeat (updated by keya_task), so no dedicated WAS task is needed.

[[noreturn]] void imu_task(void *pv_parameters) {
    for (;;) {
        hw::BNO08XIMU::handler();
        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz update rate
    }
}

[[noreturn]] void buttons_task(void *pv_parameters) {
    for (;;) {
        buttons::handler();
        vTaskDelay(pdMS_TO_TICKS(100)); // 10Hz update rate
    }
}

[[noreturn]] void autoSteerTask(void *pv_parameters) {
    for (;;) {
        autosteer::handler();
        vTaskDelay(pdMS_TO_TICKS(1)); // 1kHz update rate
    }
}

[[noreturn]] void gpsTask(void *pv_parameters) {
    for (;;) {
        gps_main::handler();
        vTaskDelay(pdMS_TO_TICKS(1)); // 1kHz update rate
    }
}

[[noreturn]] void headingTask(void *pv_parameters) {
    for (;;) {
        gps_heading::handler();
        vTaskDelay(pdMS_TO_TICKS(1)); // 1kHz update rate
    }
}

#if KEYA_MOTOR
[[noreturn]] void keya_task(void *pv_parameters) {
    for (;;) {
        hw::KeyaMotor::handler();
        // handler() blocks up to ~50ms inside twai_receive when idle, so a
        // short delay here is enough to yield without losing heartbeats.
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
#endif

#if SIMULATOR
[[noreturn]] void sim_task(void *pv_parameters) {
    TickType_t lastWake = xTaskGetTickCount();
    int nmeaCounter = 0;
    for (;;) {
        sim::tick();
        if (++nmeaCounter >= SIM_NMEA_DIVIDER) {
            nmeaCounter = 0;
            sim::emitNMEA();
        }
        vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(SIM_TICK_MS));
    }
}
#endif


bool create_tasks() {
    debug("Creating tasks...");
    BaseType_t taskCreated;

    // Safety task first - it monitors autosteer-handler heartbeat and
    // forces motor stop on stale guidance / wedged control loop. Highest
    // priority of anything that touches the motor (above buttons=6,
    // autosteer=5).
    if (!safety::init()) {
        error("Failed to create safety task");
        return false;
    }

#if !KEYA_WAS
    debug("Creating WAS task...");
    TaskHandle_t wasTaskHandle = nullptr;
    taskCreated = xTaskCreate(
        was_task,
        "was_task",
        4096,
        nullptr,
        WAS_TASK_PRIORITY,
        &wasTaskHandle
    );

    if (taskCreated != pdPASS || wasTaskHandle == nullptr) {
        error("Failed to create WAS task");
        return false;
    }
#else
    debug("WAS task skipped (KEYA_WAS=1, encoder is pulled from heartbeat)");
#endif

    delay(100);
    debug("Creating IMU task...");
    TaskHandle_t imuTaskHandle = nullptr;
    taskCreated = xTaskCreate(
        imu_task,
        "imu_task",
        4096,
        nullptr,
        IMU_TASK_PRIORITY,
        &imuTaskHandle
    );
    
    if (taskCreated != pdPASS || imuTaskHandle == nullptr) {
        error("Failed to create IMU task");
        return false;
    }

    delay(100);
    debug("Creating buttons task...");
    TaskHandle_t buttonsTaskHandle = nullptr;
    taskCreated = xTaskCreate(
        buttons_task,
        "buttons_task",
        4096,    // 1024 was too small once buttons::handler started
                 // logging via infof - vsnprintf overflowed and panicked
                 // on the first transition. 4096 matches the other tasks.
        nullptr,
        BUTTONS_TASK_PRIORITY,
        &buttonsTaskHandle
    );

    if (taskCreated != pdPASS || buttonsTaskHandle == nullptr) {
        error("Failed to create buttons task");
        return false;
    }

    delay(100);
    debug("Creating autoSteer task...");
    TaskHandle_t autoSteerTaskHandle = nullptr;
    taskCreated = xTaskCreate(
          autoSteerTask,
          "autoSteerTask",
          4096,
          NULL,
          AUTOSTEER_TASK_PRIORITY,
          &autoSteerTaskHandle
        );
    if (taskCreated != pdPASS || autoSteerTaskHandle == nullptr) {
        error("Failed to create autoSteer task");
        return false;
    }

#if SIMULATOR
    delay(100);
    debug("Creating SIM task...");
    TaskHandle_t simTaskHandle = nullptr;
    taskCreated = xTaskCreate(
        sim_task,
        "sim_task",
        4096,
        nullptr,
        SIM_TASK_PRIORITY,
        &simTaskHandle
    );
    if (taskCreated != pdPASS || simTaskHandle == nullptr) {
        error("Failed to create SIM task");
        return false;
    }
#else
    delay(100);
    debug("Creating MAIN_GPS task...");
    TaskHandle_t gpsTaskHandle = nullptr;
    taskCreated = xTaskCreate(
        gpsTask,
        "gpsTask",
        4096,
        NULL,
        GPS_TASK_PRIORITY,
        &gpsTaskHandle
    );
    if (taskCreated != pdPASS || gpsTaskHandle == nullptr) {
        error("Failed to create GPS task");
        return false;
    }
#endif
#if GPS_HEADING
    delay(100);
    debug("Creating HEADING_GPS task...");
    TaskHandle_t headingTaskHandle = nullptr;
    taskCreated = xTaskCreate(
        headingTask,
        "headingTask",  // Fixed typo: was "headerTask"
        10000,
        NULL,
        HEADING_TASK_PRIORITY,
        &headingTaskHandle
    );

    if (!taskCreated) {
        error("Failed to create HEADING_GPS task");
        return false;
    }
#endif

#if KEYA_MOTOR
    delay(100);
    debug("Creating Keya CAN task...");
    TaskHandle_t keyaTaskHandle = nullptr;
    taskCreated = xTaskCreate(
        keya_task,
        "keya_task",
        4096,
        nullptr,
        KEYA_TASK_PRIORITY,
        &keyaTaskHandle
    );
    if (taskCreated != pdPASS || keyaTaskHandle == nullptr) {
        error("Failed to create Keya task");
        return false;
    }
#endif

#if TEST_MODE
    delay(100);
    debug("Creating HW test task...");
    TaskHandle_t testTaskHandle = nullptr;
    taskCreated = xTaskCreate(
        hw::test::task,
        "hw_test",
        4096,
        nullptr,
        2,                  // low priority - it's just a logger
        &testTaskHandle
    );
    if (taskCreated != pdPASS || testTaskHandle == nullptr) {
        error("Failed to create HW test task");
        return false;
    }
#endif

    return true;

}


