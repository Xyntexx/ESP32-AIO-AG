//
// Created by Markus on 14.5.2025.
//

#ifndef HEADINGGPS_H
#define HEADINGGPS_H
#include "autosteer/networking.h"

namespace gps_heading {
bool init();
void handler();
void initGpsCommunication(bool (*send_func)(const uint8_t *, size_t), const ip_address &deviceIP);
}
#endif //HEADINGGPS_H
