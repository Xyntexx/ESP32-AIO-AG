#ifndef HW_TEST_H
#define HW_TEST_H

#include "config/defines.h"

#if TEST_MODE

namespace hw {
namespace test {

// FreeRTOS task that periodically logs the live state of every subsystem
// over the UDP debug stream. Created from create_tasks() when TEST_MODE=1.
[[noreturn]] void task(void *pv);

} // namespace test
} // namespace hw

#endif // TEST_MODE

#endif // HW_TEST_H
