#include "was.h"
#include "config/pinout.h"
#include "config/defines.h"
#include "hardware/i2c_manager.h"
#include "hardware/ADS1115/ADS1115_lite.h"

#include "settings/settings.h"
#include "utils/log.h"

namespace was {
int16_t actual_steer_pos_raw = 0; // stored for >zero< Funktion

ADS1115_lite ads1115;

//loop for WAS
// Loop triggers every 20 msec
[[noreturn]] void wheel_angle_sensor_task(void *pv_parameters) {
    TickType_t last_wake_time              = xTaskGetTickCount();
    constexpr TickType_t trigger_increment = pdMS_TO_TICKS(20);

    // Start the first conversion for the first read
    if (Set.wasType == WASType::ADS_1115_single || Set.wasType == WASType::ADS_1115_diff) {
        I2C_MUTEX_LOCK();
        if (Set.wasType == WASType::ADS_1115_single) {
            ads1115.setMux(ADS1115_MUX_SINGLE_0);
        } else {
            ads1115.setMux(ADS1115_MUX_DIFF_0_1);
        }
        ads1115.startConversion(); // Start first conversion
        I2C_MUTEX_UNLOCK();
    }

    // Wait a bit for the first conversion to complete
    vTaskDelay(pdMS_TO_TICKS(10));

    for (;;) {
        //TODO: Missing functions:
        //SetRelays(); //turn on off sections, do in timed loop, if new data comes in
        //encDebounce = LOW; //reset steerEncoder debounce

        //steering position and steer angle
        switch (Set.wasType) {
            case WASType::ADS_1115_single: // ADS 1115 single
                I2C_MUTEX_LOCK();
                // First read the result of the previous conversion
                actual_steer_pos_raw = ads1115.readConversion() >> 1; //divide by 2
                
                // Then start the next conversion
                ads1115.setMux(ADS1115_MUX_SINGLE_0);
                ads1115.startConversion();
                I2C_MUTEX_UNLOCK();
                break;
                
            case WASType::ADS_1115_diff: // ADS 1115 differential
                I2C_MUTEX_LOCK();
                // First read the result of the previous conversion
                actual_steer_pos_raw = ads1115.readConversion() >> 1; //divide by 2
                
                // Then start the next conversion
                ads1115.setMux(ADS1115_MUX_DIFF_0_1);
                ads1115.startConversion();
                I2C_MUTEX_UNLOCK();
                break;
                
            default: // directly to arduino
                warning("WAS type not supported");
                break;
        }
        xTaskDelayUntil(&last_wake_time, trigger_increment);
    }
}

int16_t get_raw_steering_position() {
    return actual_steer_pos_raw;
}

int16_t get_steering_position() {
    //center the steering position sensor
    auto steering_position = get_raw_steering_position() - Set.steerAngleOffset;

    //invert position, left must be minus
    if (Set.invertWAS == 1) steering_position *= -1;

    //Ackermann fix: correct non-linear values = right side factor
    if (steering_position > 0) {
        steering_position = long((steering_position * Set.ackermanFix) / 100.0);
    }

    return steering_position;
}

//convert position to steer angle
float get_steering_angle() {
    //center the steering position sensor
    return ((float) (get_steering_position()) / Set.steerSensorCounts);
}

// For AOG communication - get 8-bit raw wheel angle sensor value
uint8_t get_wheel_angle_sensor_raw() {
    return static_cast<uint8_t>(get_raw_steering_position() & 0xFF);
}

// Implementation of get_wheel_angle_sensor_counts
uint16_t get_wheel_angle_sensor_counts() {
    // Return the raw steering position as counts
    return static_cast<uint16_t>(get_raw_steering_position());
}

bool init() {
    bool success = true;
    
    if (Set.wasType == WASType::ADS_1115_single || Set.wasType == WASType::ADS_1115_diff) {
        debug("Initializing ADS1115 WAS");
        I2C_MUTEX_LOCK();
        Wire.end();
        Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
        Wire.begin();

        if (ads1115.isConnected()) {
            debug("WAS ADC Connection OK");
        } else {
            error("WAS ADC Connection FAILED!");
            success = false;
        }
        
        if (success) {
            ads1115.setSampleRate(ADS1115_DR_128SPS); // Set sample rate to 128SPS
            ads1115.setGain(ADS1115_GAIN_6_144V); // Set gain to 6.144V
        }
        
        I2C_MUTEX_UNLOCK();
    } else {
        warning("WAS type not supported");
        success = false;
    }

    if (success) {
        TaskHandle_t wasTaskHandle = NULL;
        BaseType_t taskCreated = xTaskCreate(
            wheel_angle_sensor_task,
            "was_task", 
            4096, 
            nullptr, 
            WAS_TASK_PRIORITY, 
            &wasTaskHandle
        );
        
        if (taskCreated != pdPASS || wasTaskHandle == NULL) {
            error("Failed to create WAS task");
            success = false;
        }
    }
    
    return success;
}
}
