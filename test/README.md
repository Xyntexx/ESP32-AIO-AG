# ESP32-AIO-AG Test Suite

Comprehensive test suite for the ESP32-AIO-AG autosteer system, covering critical bug fixes and core functionality.

## Overview

This test suite uses the [PlatformIO Unit Testing framework](https://docs.platformio.org/en/latest/advanced/unit-testing/index.html) with the Unity test framework.

## Test Coverage

### 1. Settings Validation Tests (`test_settings_validation`)

Tests the input validation logic for steer settings to ensure all parameters stay within safe ranges.

**Tests:**
- Valid settings pass through unchanged
- PWM values clamped to 0-255 range
- PID gains validated
- Min/Max PWM values automatically swapped if inverted
- Ackerman fix constrained to 50-150% range
- Sensor counts validated (non-zero)
- Multiple invalid values corrected simultaneously

**Fixes Tested:**
- Commit e49a66f - Settings validation implementation

**Run:**
```bash
pio test -e native -f test_settings_validation
```

---

### 2. Angle Decoding Tests (`test_angle_decoding`)

Tests the steering angle decoding from AgOpenGPS protocol (int16_t * 100 encoding).

**Tests:**
- Zero angle (center position)
- Positive angles (+1° to +360°)
- Negative angles (-1° to -360°)
- Fractional angles (0.01° precision)
- Out-of-range detection (> ±360°)
- Boundary conditions
- Typical steering range (-45° to +45°)
- Protocol encoding/decoding accuracy

**Fixes Tested:**
- Commit e49a66f - Proper int16 angle decoding
- Commit a463418 - Constants usage for angle limits

**Run:**
```bash
pio test -e native -f test_angle_decoding
```

---

### 3. GPS Buffer Overflow Tests (`test_gps_buffer`)

Tests the NMEA sentence parsing and buffer overflow protection.

**Tests:**
- Simple NMEA sentence parsing
- Multiple sentence handling
- Partial sentence buffering
- Buffer overflow protection
- Exact buffer size handling
- Empty data handling
- CRLF handling
- Realistic NMEA streams (byte-by-byte reception)
- Sentence boundary detection

**Fixes Tested:**
- Commit 434e637 - NMEA framing and buffer overflow protection

**Run:**
```bash
pio test -e native -f test_gps_buffer
```

---

### 4. I2C Mutex Tests (`test_i2c_mutex`)

Tests the I2C mutex locking logic to prevent race conditions.

**Tests:**
- Mutex lock succeeds when unlocked
- Mutex lock fails when already locked
- Mutex unlock succeeds when locked
- Mutex unlock fails when already unlocked
- Lock-unlock sequence
- Multiple lock-unlock cycles
- Typical I2C operation pattern
- Nested lock attempts (should fail)
- Correct macro comparison (== not !=)
- Concurrent access detection

**Fixes Tested:**
- Commit 63f806c - I2C mutex logic inversion fix

**Run:**
```bash
pio test -e native -f test_i2c_mutex
```

---

## Running Tests

### Run All Tests

```bash
pio test -e native
```

### Run Specific Test

```bash
pio test -e native -f <test_name>
```

### Run Tests with Verbose Output

```bash
pio test -e native -v
```

### Run Tests on Hardware (ESP32)

```bash
pio test -e esp32-s3-devkitm-1
```

## Test Results

All tests should pass with the current code-fixes branch. Expected output:

```
test/test_settings_validation/test_main.cpp:XXX:test_valid_settings_pass [PASSED]
test/test_settings_validation/test_main.cpp:XXX:test_high_pwm_clamping [PASSED]
...
----------------------
8 Tests 0 Failures 0 Ignored
OK
```

## Test Structure

Each test directory contains:
- `test_main.cpp` - Test implementation
- Mock implementations of dependencies
- Unity test runner

Tests are designed to run on:
- **Native** platform (x86/x64) for fast CI/CD
- **Embedded** platform (ESP32) for hardware validation

## Writing New Tests

### Example Test Structure

```cpp
#include <unity.h>

void test_feature() {
    // Arrange
    int expected = 42;

    // Act
    int actual = myFunction();

    // Assert
    TEST_ASSERT_EQUAL(expected, actual);
}

void setUp(void) {
    // Runs before each test
}

void tearDown(void) {
    // Runs after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_feature);
    return UNITY_END();
}
```

### Unity Assertions

Common assertions used:
- `TEST_ASSERT_TRUE(condition)`
- `TEST_ASSERT_FALSE(condition)`
- `TEST_ASSERT_EQUAL(expected, actual)`
- `TEST_ASSERT_FLOAT_WITHIN(delta, expected, actual)`
- `TEST_ASSERT_EQUAL_STRING(expected, actual)`

Full list: [Unity Assertions Reference](https://github.com/ThrowTheSwitch/Unity/blob/master/docs/UnityAssertionsReference.md)

## Continuous Integration

Tests can be integrated into CI/CD pipeline:

```yaml
# .github/workflows/test.yml
- name: Run Tests
  run: pio test -e native
```

## Test Coverage Summary

| Test Suite | Tests | Status | Coverage |
|------------|-------|--------|----------|
| Settings Validation | 8 | ✅ Pass | Input validation logic |
| Angle Decoding | 15 | ✅ Pass | Protocol decoding |
| GPS Buffer | 11 | ✅ Pass | NMEA parsing, overflow |
| I2C Mutex | 10 | ✅ Pass | Thread safety |
| **Total** | **44** | ✅ **Pass** | **Critical fixes** |

## Future Test Additions

Planned test suites:
- [ ] PID controller tests
- [ ] Motor control tests
- [ ] IMU data mutex tests
- [ ] UDP packet parsing tests
- [ ] Switch byte encoding tests
- [ ] Hardware initialization tests
- [ ] Stack overflow detection tests

## Test-Driven Development

When adding new features:
1. Write failing tests first
2. Implement feature
3. Ensure tests pass
4. Refactor with confidence

## Debugging Tests

### Enable Verbose Output

```bash
pio test -e native -v
```

### Run Single Test

Add `UNITY_INCLUDE_ONLY` before test names:

```cpp
#define UNITY_INCLUDE_ONLY test_my_specific_test
```

### Print Debug Info

```cpp
void test_with_debug() {
    int value = 42;
    TEST_MESSAGE("Debug value:");
    TEST_PRINTF("%d", value);
    TEST_ASSERT_EQUAL(42, value);
}
```

## References

- [PlatformIO Unit Testing](https://docs.platformio.org/en/latest/advanced/unit-testing/index.html)
- [Unity Test Framework](https://github.com/ThrowTheSwitch/Unity)
- [ESP32-ETH-NTRIP Test Examples](../../../ESP32-ETH-NTRIP/test/)

---

**Last Updated:** 2025-10-28
**Test Framework:** Unity v2.5.2
**Platform:** PlatformIO
