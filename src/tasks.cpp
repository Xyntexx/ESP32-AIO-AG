#include "tasks.h"

#include "autosteer/autosteer.h"
#include "autosteer/buttons.h"
#include "hardware/was/ads1115_was.h"
#include "hardware/imu/bno08x_imu.h"
#include "utils/log.h"

[[noreturn]] void was_task(void *pv_parameters) {
    for (;;) {
        //hw::ADS1115WAS::handler();
        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz update rate
    }
}

[[noreturn]] void imu_task(void *pv_parameters) {
    for (;;) {
        //hw::BNO08XIMU::handler();
        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz update rate
    }
}

[[noreturn]] void buttons_task(void *pv_parameters) {
    for (;;) {
        //buttons::handler();
        vTaskDelay(pdMS_TO_TICKS(100)); // 10Hz update rate
    }
}

[[noreturn]] void autoSteerTask(void *pv_parameters) {
    for (;;) {
        autosteer::handler();
        vTaskDelay(pdMS_TO_TICKS(1)); // 1kHz update rate
    }
}


bool create_tasks() {
    debug("Creating tasks...");
    debug("Creating WAS task...");
    TaskHandle_t wasTaskHandle = nullptr;
    BaseType_t taskCreated = xTaskCreate(
        was_task,
        "was_task", 
        2048,
        nullptr, 
        WAS_TASK_PRIORITY, 
        &wasTaskHandle
    );
    
    if (taskCreated != pdPASS || wasTaskHandle == nullptr) {
        error("Failed to create WAS task");
        return false;
    }

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
        1024,
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
    }
    
    return true;
}


