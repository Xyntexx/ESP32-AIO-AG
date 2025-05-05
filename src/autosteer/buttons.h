//
// Created by Markus on 5.5.2025.
//

#ifndef BUTTONS_H
#define BUTTONS_H

#include <Arduino.h>

namespace buttons
{
  // Initialize button subsystem
  void init();
  
  // Status functions
  bool steer_button_enabled();

}

#endif //BUTTONS_H
