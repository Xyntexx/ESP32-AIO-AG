#include <unity.h>
#include <cstdint>
#include <cmath>

#include "../../src/config/constants.h"

// Mock warning function
void warningf(const char* fmt, ...) { (void)fmt; }

// Function to decode steering angle (from udp_io.cpp)
float decodeSteeringAngle(uint16_t encodedAngle, bool* outOfRange = nullptr) {
    // Decode steering angle (int16_t encoded as uint16_t * scale factor)
    int16_t raw_angle = static_cast<int16_t>(encodedAngle);
    float steerAngleSetPoint = static_cast<float>(raw_angle) * ANGLE_SCALE_FACTOR;

    // Validate angle is within expected range
    if (steerAngleSetPoint < MIN_STEER_ANGLE_DEG || steerAngleSetPoint > MAX_STEER_ANGLE_DEG) {
        if (outOfRange) *outOfRange = true;
        steerAngleSetPoint = 0.0f;  // Default to center
    } else {
        if (outOfRange) *outOfRange = false;
    }

    return steerAngleSetPoint;
}

// Test: Zero angle (center position)
void test_zero_angle() {
    uint16_t encoded = 0;  // 0 * 100
    float angle = decodeSteeringAngle(encoded);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 0.0f, angle);
}

// Test: Positive angle (+10 degrees)
void test_positive_angle() {
    uint16_t encoded = 1000;  // 10.00 * 100
    float angle = decodeSteeringAngle(encoded);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 10.0f, angle);
}

// Test: Negative angle (-10 degrees)
void test_negative_angle() {
    int16_t raw = -1000;  // -10.00 * 100
    uint16_t encoded = static_cast<uint16_t>(raw);
    float angle = decodeSteeringAngle(encoded);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, -10.0f, angle);
}

// Test: Maximum positive angle (+327.67 degrees - int16_t max)
void test_max_positive_angle() {
    uint16_t encoded = 32767;  // 327.67 * 100 (max int16_t)
    float angle = decodeSteeringAngle(encoded);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 327.67f, angle);
}

// Test: Maximum negative angle (-327.68 degrees - int16_t min)
void test_max_negative_angle() {
    int16_t raw = -32768;  // -327.68 * 100 (min int16_t)
    uint16_t encoded = static_cast<uint16_t>(raw);
    float angle = decodeSteeringAngle(encoded);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, -327.68f, angle);
}

// Test: Small positive angle (+1 degree)
void test_small_positive_angle() {
    uint16_t encoded = 100;  // 1.00 * 100
    float angle = decodeSteeringAngle(encoded);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 1.0f, angle);
}

// Test: Small negative angle (-1 degree)
void test_small_negative_angle() {
    int16_t raw = -100;  // -1.00 * 100
    uint16_t encoded = static_cast<uint16_t>(raw);
    float angle = decodeSteeringAngle(encoded);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, -1.0f, angle);
}

// Test: Fractional angle (+45.5 degrees)
void test_fractional_angle() {
    uint16_t encoded = 4550;  // 45.50 * 100
    float angle = decodeSteeringAngle(encoded);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 45.5f, angle);
}

// Test: Negative fractional angle (-127.25 degrees)
void test_negative_fractional_angle() {
    int16_t raw = -12725;  // -127.25 * 100
    uint16_t encoded = static_cast<uint16_t>(raw);
    float angle = decodeSteeringAngle(encoded);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, -127.25f, angle);
}

// Test: Values within range should not trigger out of range
void test_out_of_range_positive() {
    uint16_t encoded = 30000;  // 300.00 * 100 (< 360, within range)
    bool outOfRange = false;
    float angle = decodeSteeringAngle(encoded, &outOfRange);

    TEST_ASSERT_FALSE(outOfRange);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 300.0f, angle);
}

// Test: Large negative values within int16 range
void test_out_of_range_negative() {
    int16_t raw = -30000;  // -300.00 * 100 (> -360, within range)
    uint16_t encoded = static_cast<uint16_t>(raw);
    bool outOfRange = false;
    float angle = decodeSteeringAngle(encoded, &outOfRange);

    TEST_ASSERT_FALSE(outOfRange);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -300.0f, angle);
}

// Test: Boundary case - within positive limit
void test_boundary_positive_limit() {
    uint16_t encoded = 32000;  // 320.00 degrees (< 360, within range)
    bool outOfRange = false;
    float angle = decodeSteeringAngle(encoded, &outOfRange);

    TEST_ASSERT_FALSE(outOfRange);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, 320.0f, angle);
}

// Test: Boundary case - within negative limit
void test_boundary_negative_limit() {
    int16_t raw = -32000;  // -320.00 degrees (> -360, within range)
    uint16_t encoded = static_cast<uint16_t>(raw);
    bool outOfRange = false;
    float angle = decodeSteeringAngle(encoded, &outOfRange);

    TEST_ASSERT_FALSE(outOfRange);
    TEST_ASSERT_FLOAT_WITHIN(0.001f, -320.0f, angle);
}

// Test: Precision test - 0.01 degree increments
void test_precision() {
    uint16_t encoded = 1234;  // 12.34 * 100
    float angle = decodeSteeringAngle(encoded);

    TEST_ASSERT_FLOAT_WITHIN(0.001f, 12.34f, angle);
}

// Test: Range of typical steering angles
void test_typical_steering_range() {
    // Test -45 to +45 degrees (typical vehicle steering range)
    for (int deg = -45; deg <= 45; deg++) {
        int16_t raw = deg * 100;
        uint16_t encoded = static_cast<uint16_t>(raw);
        float angle = decodeSteeringAngle(encoded);

        TEST_ASSERT_FLOAT_WITHIN(0.001f, (float)deg, angle);
    }
}

void setUp(void) {
    // Set up before each test
}

void tearDown(void) {
    // Clean up after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_zero_angle);
    RUN_TEST(test_positive_angle);
    RUN_TEST(test_negative_angle);
    RUN_TEST(test_max_positive_angle);
    RUN_TEST(test_max_negative_angle);
    RUN_TEST(test_small_positive_angle);
    RUN_TEST(test_small_negative_angle);
    RUN_TEST(test_fractional_angle);
    RUN_TEST(test_negative_fractional_angle);
    RUN_TEST(test_out_of_range_positive);
    RUN_TEST(test_out_of_range_negative);
    RUN_TEST(test_boundary_positive_limit);
    RUN_TEST(test_boundary_negative_limit);
    RUN_TEST(test_precision);
    RUN_TEST(test_typical_steering_range);

    return UNITY_END();
}
