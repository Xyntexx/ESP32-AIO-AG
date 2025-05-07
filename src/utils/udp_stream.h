#pragma once

#include <Arduino.h>
#include <AsyncUDP.h>
#include <Stream.h>

#include "w6100/esp32_sc_w6100.h"

class UDPStream : public Stream {
private:
    AsyncUDP udp;
    IPAddress broadcastAddress;
    uint16_t port;
    bool initialized;
    static const size_t bufferSize = 1024; // Size of the buffer for UDP messages
    uint8_t buffer[bufferSize];  // Buffer to collect bytes for complete lines
    size_t bufferIndex = 0;

public:
    UDPStream(IPAddress broadcastAddr = IPAddress(255, 255, 255, 255), uint16_t port = 7777)
        : broadcastAddress(broadcastAddr), port(port), initialized(false) {}

    bool begin() {
        if (udp.listen(port)) {
            initialized = true;
            return true;
        }
        return false;
    }

    // Stream interface implementation
    int available() override { return 0; }  // UDP doesn't support reading in this context
    int read() override { return -1; }      // UDP doesn't support reading in this context
    int peek() override { return -1; }      // UDP doesn't support reading in this context
    void flush() override { 
        // Send any buffered data
        if (initialized && bufferIndex) {
            udp.broadcastTo((uint8_t*)buffer, bufferIndex, port);
            bufferIndex = 0; // Reset buffer index after sending
        }
    }

    // Print interface implementation
    size_t write(uint8_t c) override {
        if (initialized) {
            buffer[bufferIndex++] = c; // Add byte to buffer

            
            // If we get a newline, send the buffer
            if (c == '\n' || bufferIndex >= bufferSize) {
                udp.writeTo(buffer, bufferIndex,ETH.broadcastIP(), port);
                bufferIndex = 0; // Reset buffer index after sending
            }
            return 1;
        }
        return 0;
    }

    size_t write(uint8_t *buffer, size_t size) {
        if (initialized) {
            udp.broadcastTo(buffer, size, port);
            return size;
        }
        return 0;
    }
}; 