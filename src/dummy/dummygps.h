//
// Created by Markus on 7.5.2025.
//

#ifndef DUMMYGPS_H
#define DUMMYGPS_H
#include <cstdint>

#include "autosteer/networking.h"

namespace dummy_gps {
// Main handler function
void handler();
void init();
void initGpsCommunication(bool (*send_func)(const uint8_t *, size_t), const ip_address &deviceIP);
void process_udp_message(uint8_t *data, uint16_t len, const ip_address &deviceIP);

}

#endif //DUMMYGPS_H
