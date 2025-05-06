#ifndef TASKS_H
#define TASKS_H

#include <Arduino.h>
#include "config/defines.h"

// Task function declarations
[[noreturn]] void was_task(void *pv_parameters);
[[noreturn]] void imu_task(void *pv_parameters);
[[noreturn]] void buttons_task(void *pv_parameters);

// Task creation functions
bool create_tasks();


#endif // TASKS_H 