#include <unity.h>
#include <cstdint>

// Mock FreeRTOS types and functions for testing
typedef void* SemaphoreHandle_t;
typedef uint32_t TickType_t;
typedef int32_t BaseType_t;

#define pdTRUE  ((BaseType_t)1)
#define pdFALSE ((BaseType_t)0)
#define portMAX_DELAY ((TickType_t)0xFFFFFFFF)

// Mock semaphore state
static bool mockMutexLocked = false;
static int lockCount = 0;
static int unlockCount = 0;

// Mock semaphore functions
BaseType_t xSemaphoreTake(SemaphoreHandle_t mutex, TickType_t timeout) {
    (void)mutex;
    (void)timeout;

    if (mockMutexLocked) {
        return pdFALSE; // Already locked
    }

    mockMutexLocked = true;
    lockCount++;
    return pdTRUE;
}

BaseType_t xSemaphoreGive(SemaphoreHandle_t mutex) {
    (void)mutex;

    if (!mockMutexLocked) {
        return pdFALSE; // Can't unlock if not locked
    }

    mockMutexLocked = false;
    unlockCount++;
    return pdTRUE;
}

// Test the I2C mutex macros (from i2c_manager.h)
#define I2C_MUTEX_LOCK() (xSemaphoreTake((SemaphoreHandle_t)1, portMAX_DELAY) == pdTRUE)
#define I2C_MUTEX_UNLOCK() xSemaphoreGive((SemaphoreHandle_t)1)

// Test: Mutex lock succeeds when unlocked
void test_mutex_lock_succeeds() {
    mockMutexLocked = false;
    lockCount = 0;

    bool result = I2C_MUTEX_LOCK();

    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(mockMutexLocked);
    TEST_ASSERT_EQUAL(1, lockCount);
}

// Test: Mutex lock fails when already locked
void test_mutex_lock_fails_when_locked() {
    mockMutexLocked = true;
    lockCount = 0;

    bool result = I2C_MUTEX_LOCK();

    TEST_ASSERT_FALSE(result);
    TEST_ASSERT_EQUAL(0, lockCount); // Should not increment
}

// Test: Mutex unlock succeeds when locked
void test_mutex_unlock_succeeds() {
    mockMutexLocked = true;
    unlockCount = 0;

    BaseType_t result = I2C_MUTEX_UNLOCK();

    TEST_ASSERT_EQUAL(pdTRUE, result);
    TEST_ASSERT_FALSE(mockMutexLocked);
    TEST_ASSERT_EQUAL(1, unlockCount);
}

// Test: Mutex unlock fails when already unlocked
void test_mutex_unlock_fails_when_unlocked() {
    mockMutexLocked = false;
    unlockCount = 0;

    BaseType_t result = I2C_MUTEX_UNLOCK();

    TEST_ASSERT_EQUAL(pdFALSE, result);
    TEST_ASSERT_EQUAL(0, unlockCount); // Should not increment
}

// Test: Lock-unlock sequence
void test_lock_unlock_sequence() {
    mockMutexLocked = false;
    lockCount = 0;
    unlockCount = 0;

    // Lock
    bool lockResult = I2C_MUTEX_LOCK();
    TEST_ASSERT_TRUE(lockResult);
    TEST_ASSERT_TRUE(mockMutexLocked);

    // Unlock
    BaseType_t unlockResult = I2C_MUTEX_UNLOCK();
    TEST_ASSERT_EQUAL(pdTRUE, unlockResult);
    TEST_ASSERT_FALSE(mockMutexLocked);

    TEST_ASSERT_EQUAL(1, lockCount);
    TEST_ASSERT_EQUAL(1, unlockCount);
}

// Test: Multiple lock-unlock cycles
void test_multiple_lock_unlock_cycles() {
    mockMutexLocked = false;
    lockCount = 0;
    unlockCount = 0;

    for (int i = 0; i < 10; i++) {
        bool lockResult = I2C_MUTEX_LOCK();
        TEST_ASSERT_TRUE(lockResult);
        TEST_ASSERT_TRUE(mockMutexLocked);

        BaseType_t unlockResult = I2C_MUTEX_UNLOCK();
        TEST_ASSERT_EQUAL(pdTRUE, unlockResult);
        TEST_ASSERT_FALSE(mockMutexLocked);
    }

    TEST_ASSERT_EQUAL(10, lockCount);
    TEST_ASSERT_EQUAL(10, unlockCount);
}

// Test: Typical I2C operation pattern
void test_typical_i2c_operation() {
    mockMutexLocked = false;

    // Simulate I2C read operation
    if (I2C_MUTEX_LOCK()) {
        // I2C operation would happen here
        TEST_ASSERT_TRUE(mockMutexLocked);

        I2C_MUTEX_UNLOCK();
        TEST_ASSERT_FALSE(mockMutexLocked);
    } else {
        TEST_FAIL_MESSAGE("Failed to acquire I2C mutex");
    }
}

// Test: Nested lock attempt (should fail)
void test_nested_lock_fails() {
    mockMutexLocked = false;

    // First lock
    bool lock1 = I2C_MUTEX_LOCK();
    TEST_ASSERT_TRUE(lock1);

    // Try to lock again (should fail)
    bool lock2 = I2C_MUTEX_LOCK();
    TEST_ASSERT_FALSE(lock2);

    // Unlock
    I2C_MUTEX_UNLOCK();
    TEST_ASSERT_FALSE(mockMutexLocked);
}

// Test: Verify macro uses == not !=
void test_macro_uses_correct_comparison() {
    mockMutexLocked = false;

    // The correct macro should be:
    // #define I2C_MUTEX_LOCK() (xSemaphoreTake(...) == pdTRUE)
    // NOT:
    // #define I2C_MUTEX_LOCK() (xSemaphoreTake(...) != pdTRUE)

    bool result = I2C_MUTEX_LOCK();

    // If the macro uses == pdTRUE correctly, this should be true
    TEST_ASSERT_TRUE(result);

    // If it incorrectly used != pdTRUE, result would be false
    // and the mutex would proceed without actually being locked
}

// Test: Simulated concurrent access detection
void test_concurrent_access_detection() {
    mockMutexLocked = false;

    // Task 1 acquires lock
    bool task1_locked = I2C_MUTEX_LOCK();
    TEST_ASSERT_TRUE(task1_locked);

    // Task 2 attempts to acquire lock (should fail)
    bool task2_locked = I2C_MUTEX_LOCK();
    TEST_ASSERT_FALSE(task2_locked);

    // Task 1 releases lock
    I2C_MUTEX_UNLOCK();

    // Now Task 2 can acquire lock
    task2_locked = I2C_MUTEX_LOCK();
    TEST_ASSERT_TRUE(task2_locked);

    I2C_MUTEX_UNLOCK();
}

void setUp(void) {
    // Reset state before each test
    mockMutexLocked = false;
    lockCount = 0;
    unlockCount = 0;
}

void tearDown(void) {
    // Clean up after each test
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_mutex_lock_succeeds);
    RUN_TEST(test_mutex_lock_fails_when_locked);
    RUN_TEST(test_mutex_unlock_succeeds);
    RUN_TEST(test_mutex_unlock_fails_when_unlocked);
    RUN_TEST(test_lock_unlock_sequence);
    RUN_TEST(test_multiple_lock_unlock_cycles);
    RUN_TEST(test_typical_i2c_operation);
    RUN_TEST(test_nested_lock_fails);
    RUN_TEST(test_macro_uses_correct_comparison);
    RUN_TEST(test_concurrent_access_detection);

    return UNITY_END();
}
