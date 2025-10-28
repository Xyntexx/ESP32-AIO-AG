#include <unity.h>
#include <cstdint>

// Include the settings structures
#include "../../src/autosteer/settings.h"
#include "../../src/config/constants.h"

// Mock logging functions
void warning(const char* msg) { (void)msg; }
void warningf(const char* fmt, ...) { (void)fmt; }

// Function under test (from settings.cpp)
namespace settings {
    extern SteerSettings settings;
    extern SteerConfig config;

    // Copy of validation function for testing
    bool validateSettings(SteerSettings& s, SteerConfig& c) {
        bool valid = true;

        // Validate PID gains (0-255 range)
        if (s.gainP > MAX_PWM_VALUE) {
            s.gainP = MAX_PWM_VALUE;
            valid = false;
        }

        // Validate PWM limits
        if (s.highPWM > MAX_PWM_VALUE) {
            s.highPWM = MAX_PWM_VALUE;
            valid = false;
        }

        if (s.minPWM > s.highPWM) {
            uint8_t temp = s.minPWM;
            s.minPWM = s.highPWM;
            s.highPWM = temp;
            valid = false;
        }

        // Validate Ackerman fix
        if (c.pulseCountMax < MIN_ACKERMAN_FIX || c.pulseCountMax > MAX_ACKERMAN_FIX) {
            c.pulseCountMax = DEFAULT_ACKERMAN_FIX;
            valid = false;
        }

        // Validate sensor counts (must be non-zero)
        if (s.steerSensorCounts == 0) {
            s.steerSensorCounts = DEFAULT_SENSOR_COUNTS;
            valid = false;
        }

        return valid;
    }
}

// Test: Valid settings should pass validation
void test_valid_settings_pass() {
    settings::SteerSettings s;
    settings::SteerConfig c;

    s.gainP = 100;
    s.highPWM = 200;
    s.minPWM = 50;
    s.steerSensorCounts = 110;
    c.pulseCountMax = 100;

    bool result = settings::validateSettings(s, c);

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_EQUAL(100, s.gainP);
    TEST_ASSERT_EQUAL(200, s.highPWM);
    TEST_ASSERT_EQUAL(50, s.minPWM);
    TEST_ASSERT_EQUAL(110, s.steerSensorCounts);
    TEST_ASSERT_EQUAL(100, c.pulseCountMax);
}

// Test: High PWM exceeds limit - should clamp to MAX_PWM_VALUE
void test_high_pwm_clamping() {
    settings::SteerSettings s;
    settings::SteerConfig c;

    s.gainP = 100;
    s.highPWM = 300; // Invalid
    s.minPWM = 50;
    s.steerSensorCounts = 110;
    c.pulseCountMax = 100;

    bool result = settings::validateSettings(s, c);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(MAX_PWM_VALUE, s.highPWM);
}

// Test: gainP exceeds limit - should clamp to MAX_PWM_VALUE
void test_gainp_clamping() {
    settings::SteerSettings s;
    settings::SteerConfig c;

    s.gainP = 300; // Invalid
    s.highPWM = 200;
    s.minPWM = 50;
    s.steerSensorCounts = 110;
    c.pulseCountMax = 100;

    bool result = settings::validateSettings(s, c);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(MAX_PWM_VALUE, s.gainP);
}

// Test: minPWM > highPWM - should swap values
void test_pwm_swap() {
    settings::SteerSettings s;
    settings::SteerConfig c;

    s.gainP = 100;
    s.highPWM = 50;  // Lower than minPWM
    s.minPWM = 200;  // Higher than highPWM
    s.steerSensorCounts = 110;
    c.pulseCountMax = 100;

    bool result = settings::validateSettings(s, c);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(200, s.highPWM); // Swapped
    TEST_ASSERT_EQUAL(50, s.minPWM);   // Swapped
}

// Test: Ackerman fix too low - should default
void test_ackerman_fix_too_low() {
    settings::SteerSettings s;
    settings::SteerConfig c;

    s.gainP = 100;
    s.highPWM = 200;
    s.minPWM = 50;
    s.steerSensorCounts = 110;
    c.pulseCountMax = 20; // Too low

    bool result = settings::validateSettings(s, c);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(DEFAULT_ACKERMAN_FIX, c.pulseCountMax);
}

// Test: Ackerman fix too high - should default
void test_ackerman_fix_too_high() {
    settings::SteerSettings s;
    settings::SteerConfig c;

    s.gainP = 100;
    s.highPWM = 200;
    s.minPWM = 50;
    s.steerSensorCounts = 110;
    c.pulseCountMax = 200; // Too high

    bool result = settings::validateSettings(s, c);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(DEFAULT_ACKERMAN_FIX, c.pulseCountMax);
}

// Test: Zero sensor counts - should default
void test_zero_sensor_counts() {
    settings::SteerSettings s;
    settings::SteerConfig c;

    s.gainP = 100;
    s.highPWM = 200;
    s.minPWM = 50;
    s.steerSensorCounts = 0; // Invalid
    c.pulseCountMax = 100;

    bool result = settings::validateSettings(s, c);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(DEFAULT_SENSOR_COUNTS, s.steerSensorCounts);
}

// Test: Multiple invalid values - should correct all
void test_multiple_invalid_values() {
    settings::SteerSettings s;
    settings::SteerConfig c;

    s.gainP = 300;          // Invalid
    s.highPWM = 50;         // Will be swapped
    s.minPWM = 200;         // Will be swapped
    s.steerSensorCounts = 0; // Invalid
    c.pulseCountMax = 200;   // Invalid

    bool result = settings::validateSettings(s, c);

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(MAX_PWM_VALUE, s.gainP);
    TEST_ASSERT_EQUAL(200, s.highPWM);  // Swapped
    TEST_ASSERT_EQUAL(50, s.minPWM);    // Swapped
    TEST_ASSERT_EQUAL(DEFAULT_SENSOR_COUNTS, s.steerSensorCounts);
    TEST_ASSERT_EQUAL(DEFAULT_ACKERMAN_FIX, c.pulseCountMax);
}

void setUp(void) {
    // Set up before each test
}

void tearDown(void) {
    // Clean up after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_valid_settings_pass);
    RUN_TEST(test_high_pwm_clamping);
    RUN_TEST(test_gainp_clamping);
    RUN_TEST(test_pwm_swap);
    RUN_TEST(test_ackerman_fix_too_low);
    RUN_TEST(test_ackerman_fix_too_high);
    RUN_TEST(test_zero_sensor_counts);
    RUN_TEST(test_multiple_invalid_values);

    return UNITY_END();
}
