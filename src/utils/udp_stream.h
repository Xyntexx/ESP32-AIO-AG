#pragma once

#include <Arduino.h>
#include <AsyncUDP.h>
#include <Stream.h>

class UDPStream : public Stream {
private:
    AsyncUDP udp;
    IPAddress broadcastAddress;
    uint16_t port;
    bool initialized;
    String buffer;  // Buffer to collect bytes for complete lines

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
        if (initialized && buffer.length() > 0) {
            udp.broadcastTo((uint8_t*)buffer.c_str(), buffer.length(), port);
            buffer = "";
        }
    }

    // Print interface implementation
    size_t write(uint8_t c) override {
        if (initialized) {
            buffer += (char)c;
            
            // If we get a newline, send the buffer
            if (c == '\n') {
                udp.broadcastTo((uint8_t*)buffer.c_str(), buffer.length(), port);
                buffer = "";
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