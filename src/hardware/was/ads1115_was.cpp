#include "ads1115_was.h"
#include "../../config/pinout.h"
#include "../../config/defines.h"
#include "../../hardware/i2c_manager.h"
#include "../../utils/log.h"
#include "autosteer/was.h"

namespace hw {

ADS1115_lite ADS1115WAS::ads1115;
int16_t ADS1115WAS::actual_steer_pos_raw = 0;
bool ADS1115WAS::first_read = true;

bool ADS1115WAS::init() {
    debug("Initializing ADS1115 WAS");
    I2C_MUTEX_LOCK();
    Wire.end();
    Wire.setPins(I2C_SDA_PIN, I2C_SCL_PIN);
    Wire.begin();

    if (ads1115.isConnected()) {
        debug("WAS ADC Connection OK");
    } else {
        error("WAS ADC Connection FAILED!");
        I2C_MUTEX_UNLOCK();
        return false;
    }

    ads1115.setSampleRate(ADS1115_DR_128SPS); // Set sample rate to 128SPS
    ads1115.setGain(ADS1115_GAIN_6_144V); // Set gain to 6.144V

    // Start the first conversion for the first read
    if (was::get_type() == WASType::single) {
        ads1115.setMux(ADS1115_MUX_SINGLE_0);
    } else {
        ads1115.setMux(ADS1115_MUX_DIFF_0_1);
    }
    ads1115.startConversion(); // Start first conversion
    I2C_MUTEX_UNLOCK();

    was::WASInterface interface;
    interface.readRaw = readRaw;
    was::init(interface);
    return true;
}

int16_t ADS1115WAS::readRaw() {
    // moving 0 to 2.5V and dividing by 2.
    // range should be now +- 6_666
    return (actual_steer_pos_raw - (32767 /6.144 * 2.5))/2;
}

void ADS1115WAS::handler() {
    switch (was::get_type()) {
        case WASType::single: // ADS 1115 single
            I2C_MUTEX_LOCK();
            // First read the result of the previous conversion
            actual_steer_pos_raw = ads1115.readConversion();

            // Then start the next conversion
            ads1115.setMux(ADS1115_MUX_SINGLE_0);
            ads1115.startConversion();
            I2C_MUTEX_UNLOCK();
            break;

        case WASType::diff: // ADS 1115 differential
            I2C_MUTEX_LOCK();
            // First read the result of the previous conversion
            actual_steer_pos_raw = ads1115.readConversion();

            // Then start the next conversion
            ads1115.setMux(ADS1115_MUX_DIFF_0_1);
            ads1115.startConversion();
            I2C_MUTEX_UNLOCK();
            break;

        default:
            warning("WAS type not supported");
            break;
    }
}

} // namespace hw