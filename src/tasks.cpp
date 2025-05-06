#include "tasks.h"
#include "hardware/was/ads1115_was.h"
#include "hardware/imu/bno08x_imu.h"
#include "hardware/buttons/buttons.h"
#include "utils/log.h"
#include "autosteer/buttons.h"
#include "autosteer/steer_logic.h"
#include "settings/settings.h"

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

[[noreturn]] void buttons_task(void *pv_parameters) {
    bool prev_momentary_state = false;
    bool sw_switch_valid = false;
    bool steer_enable = false;
    
    // Initial delay to allow other systems to initialize
    vTaskDelay(pdMS_TO_TICKS(100));
    
    for (;;) {
        const bool btn_state = hw::Buttons::isAutoSteerEnabled();
        
        switch (autosteer::steer_switch_type_types::momentary) { // Replace with actual settings when available
            case autosteer::steer_switch_type_types::SWITCH:// Simple switch state follows the button directly
                steer_enable = btn_state;
                break;
                
            case autosteer::steer_switch_type_types::momentary:// Toggle on button release (when it was previously pressed)
                if (!btn_state && prev_momentary_state) {
                    steer_enable = !steer_enable;
                }
                prev_momentary_state = btn_state;
                break;
                
            case autosteer::steer_switch_type_types::none:
                // No physical switch - use software switch when valid guidance data is available
                if (!autosteer::guidancePacketValid()) {
                    steer_enable = false;
                } else if (sw_switch_valid) {
                    steer_enable = autosteer::getSwSwitchStatus();
                } else if (!autosteer::getSwSwitchStatus()) {
                    sw_switch_valid = true;
                }
                break;
                
            default:
                // Analog and any unknown types default to disabled
                steer_enable = false;
        }
        
        vTaskDelay(pdMS_TO_TICKS(100)); // 10Hz update rate
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
        error("Failed to create IMU task");
        return false;
    }

    TaskHandle_t buttonsTaskHandle = nullptr;
    taskCreated = xTaskCreate(
        buttons_task,
        "buttons_task",
        2048,
        nullptr,
        BUTTONS_TASK_PRIORITY,
        &buttonsTaskHandle
    );
    
    if (taskCreated != pdPASS || buttonsTaskHandle == nullptr) {
        error("Failed to create buttons task");
        return false;
    }
    
    return true;
}


