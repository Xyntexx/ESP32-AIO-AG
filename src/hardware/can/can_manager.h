#ifndef CAN_MANAGER_H
#define CAN_MANAGER_H

#include "config/defines.h"

#if KEYA_MOTOR

#include <Arduino.h>
#include "driver/twai.h"

namespace hw {
namespace can {

bool init();

bool send(const twai_message_t& msg, TickType_t timeout = pdMS_TO_TICKS(20));

bool receive(twai_message_t& msg, TickType_t timeout = pdMS_TO_TICKS(50));

[[noreturn]] void task(void* pvParameters);

} // namespace can
} // namespace hw

#endif // KEYA_MOTOR

#endif // CAN_MANAGER_H
