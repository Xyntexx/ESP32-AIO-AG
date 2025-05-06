#include "tasks.h"
#include "hardware/was/ads1115_was.h"
#include "hardware/imu/bno08x_imu.h"
#include "utils/log.h"

[[noreturn]] void was_task(void *pv_parameters) {
    // Initial delay to allow other systems to initialize
    vTaskDelay(pdMS_TO_TICKS(100));

    for (;;) {
        hw::ADS1115WAS::handler();
        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz update rate
    }
}

[[noreturn]] void imu_task(void *pv_parameters) {
    // Initial delay to allow other systems to initialize
    vTaskDelay(pdMS_TO_TICKS(100));

    for (;;) {
        hw::BNO08XIMU::handler();
        vTaskDelay(pdMS_TO_TICKS(20)); // 50Hz update rate
    }
}

bool create_tasks() {
    TaskHandle_t wasTaskHandle = nullptr;
    BaseType_t taskCreated = xTaskCreate(
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
        error("Failed to create WAS task");
        return false;
    }

    
    return true;
}


