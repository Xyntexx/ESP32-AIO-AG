#include "buttons.h"

#include "config/pinout.h"
#include "config/defines.h"
#include "settings/settings.h"
#include "utils/log.h"
#include "udp_io.h"

namespace buttons {

namespace {
bool steer_enable = false;

[[noreturn]] void buttons_task(void *pv_parameters) {
  bool prev_momentary_state = false;
  bool sw_switch_valid = false;
  
  for (;;) {
    const bool btn_state = digitalRead(STEER_BTN_PIN) == STEER_BTN_ACTIVE_STATE;
    
    switch (Set.steer_switch_type) {
      case steer_switch_type_types::SWITCH:// Simple switch state follows the button directly
        steer_enable = btn_state;
        break;
        
      case steer_switch_type_types::momentary:// Toggle on button release (when it was previously pressed)
        if (!btn_state && prev_momentary_state) {
          steer_enable = !steer_enable;
        }
        prev_momentary_state = btn_state;
        break;
        
      case steer_switch_type_types::none:
        // No physical switch - use software switch when valid guidance data is available
        if (!guidancePacketValid()) {
          steer_enable = false;
        } else if (sw_switch_valid) {
          steer_enable = getSwSwitchStatus();
        } else if (!getSwSwitchStatus()) {
          sw_switch_valid = true;
        }
        break;
        
      default:
        // Analog and any unknown types default to disabled
        steer_enable = false;
    }
    
    delay(100);
  }
}
} // end anonymous namespace

bool steer_button_enabled() {
  return steer_enable;
}

bool init() {
  debug("Initializing buttons");
  pinMode(STEER_BTN_PIN, INPUT);
  
  TaskHandle_t buttonsTaskHandle = NULL;
  BaseType_t taskCreated = xTaskCreate(
    buttons_task, 
    "buttons_task", 
    2048, 
    nullptr, 
    BUTTONS_TASK_PRIORITY, 
    &buttonsTaskHandle
  );
  
  if (taskCreated != pdPASS || buttonsTaskHandle == NULL) {
    error("Failed to create buttons task");
    return false;
  }
  
  debug("Buttons initialized successfully");
  return true;
}
} // end namespace buttons


