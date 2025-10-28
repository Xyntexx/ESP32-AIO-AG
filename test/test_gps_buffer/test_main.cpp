#include <unity.h>
#include <cstdint>
#include <cstring>
#include <vector>

#include "../../src/config/constants.h"

// Mock error function
void error(const char* msg) { (void)msg; }

// Mock UDP send function
static std::vector<std::vector<uint8_t>> sentPackets;
static bool udpSendMock(const uint8_t* data, size_t len) {
    std::vector<uint8_t> packet(data, data + len);
    sentPackets.push_back(packet);
    return true;
}

// Simulated GPS buffer handler (from gps_module.cpp)
class GPSBufferHandler {
private:
    static constexpr size_t buffer_size = GPS_BUFFER_SIZE;
    uint8_t buffer[buffer_size];
    int buffer_pos;
    bool (*udp_send_func)(const uint8_t*, size_t);

public:
    GPSBufferHandler() : buffer_pos(0), udp_send_func(nullptr) {
        memset(buffer, 0, buffer_size);
    }

    void setUDPSender(bool (*func)(const uint8_t*, size_t)) {
        udp_send_func = func;
    }

    // Process incoming GPS data (NMEA sentences)
    void processData(const char* data, size_t len) {
        if (!udp_send_func) return;

        for (size_t i = 0; i < len; i++) {
            char c = data[i];

            // Check for buffer overflow before adding character
            if (buffer_pos >= buffer_size - 1) {
                error("GPS buffer overflow, resetting buffer");
                buffer_pos = 0;
                break;
            }

            buffer[buffer_pos++] = c;

            // NMEA sentences end with newline - send complete sentence
            if (c == '\n') {
                udp_send_func(buffer, buffer_pos);
                buffer_pos = 0;
                break; // Process one sentence per call
            }
        }
    }

    int getBufferPos() const { return buffer_pos; }
    void reset() { buffer_pos = 0; sentPackets.clear(); }
};

static GPSBufferHandler handler;

// Test: Simple NMEA sentence
void test_simple_nmea_sentence() {
    handler.reset();
    handler.setUDPSender(udpSendMock);

    const char* sentence = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n";
    handler.processData(sentence, strlen(sentence));

    TEST_ASSERT_EQUAL(1, sentPackets.size());
    TEST_ASSERT_EQUAL(strlen(sentence), sentPackets[0].size());
    TEST_ASSERT_EQUAL(0, handler.getBufferPos()); // Buffer should be reset
}

// Test: Multiple NMEA sentences
void test_multiple_sentences() {
    handler.reset();
    handler.setUDPSender(udpSendMock);

    const char* sentence1 = "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n";
    const char* sentence2 = "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39\r\n";

    handler.processData(sentence1, strlen(sentence1));
    handler.processData(sentence2, strlen(sentence2));

    TEST_ASSERT_EQUAL(2, sentPackets.size());
    TEST_ASSERT_EQUAL(0, handler.getBufferPos());
}

// Test: Partial sentence (no newline yet)
void test_partial_sentence() {
    handler.reset();
    handler.setUDPSender(udpSendMock);

    const char* partial = "$GPGGA,123519,4807.038";
    handler.processData(partial, strlen(partial));

    TEST_ASSERT_EQUAL(0, sentPackets.size()); // Nothing sent yet
    TEST_ASSERT_EQUAL(strlen(partial), handler.getBufferPos()); // Data buffered
}

// Test: Partial then complete sentence
void test_partial_then_complete() {
    handler.reset();
    handler.setUDPSender(udpSendMock);

    const char* partial1 = "$GPGGA,123519,";
    const char* partial2 = "4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n";

    handler.processData(partial1, strlen(partial1));
    TEST_ASSERT_EQUAL(0, sentPackets.size());

    handler.processData(partial2, strlen(partial2));
    TEST_ASSERT_EQUAL(1, sentPackets.size());
    TEST_ASSERT_EQUAL(strlen(partial1) + strlen(partial2), sentPackets[0].size());
}

// Test: Buffer overflow protection
void test_buffer_overflow_protection() {
    handler.reset();
    handler.setUDPSender(udpSendMock);

    // Create a string longer than buffer size without newline
    char longData[GPS_BUFFER_SIZE + 100];
    memset(longData, 'A', sizeof(longData));
    longData[sizeof(longData) - 1] = '\0';

    handler.processData(longData, strlen(longData));

    // Buffer should have been reset on overflow
    TEST_ASSERT_EQUAL(0, handler.getBufferPos());
    TEST_ASSERT_EQUAL(0, sentPackets.size()); // Nothing should be sent
}

// Test: Exact buffer size sentence
void test_exact_buffer_size() {
    handler.reset();
    handler.setUDPSender(udpSendMock);

    // Create sentence that's exactly buffer_size - 1 (leaving room for newline)
    char exactSize[GPS_BUFFER_SIZE];
    memset(exactSize, 'B', GPS_BUFFER_SIZE - 2);
    exactSize[GPS_BUFFER_SIZE - 2] = '\n';
    exactSize[GPS_BUFFER_SIZE - 1] = '\0';

    handler.processData(exactSize, strlen(exactSize));

    TEST_ASSERT_EQUAL(1, sentPackets.size());
    TEST_ASSERT_EQUAL(GPS_BUFFER_SIZE - 1, sentPackets[0].size());
}

// Test: Empty data
void test_empty_data() {
    handler.reset();
    handler.setUDPSender(udpSendMock);

    handler.processData("", 0);

    TEST_ASSERT_EQUAL(0, sentPackets.size());
    TEST_ASSERT_EQUAL(0, handler.getBufferPos());
}

// Test: Only newline
void test_only_newline() {
    handler.reset();
    handler.setUDPSender(udpSendMock);

    handler.processData("\n", 1);

    TEST_ASSERT_EQUAL(1, sentPackets.size());
    TEST_ASSERT_EQUAL(1, sentPackets[0].size());
    TEST_ASSERT_EQUAL('\n', sentPackets[0][0]);
}

// Test: Multiple newlines
void test_multiple_newlines() {
    handler.reset();
    handler.setUDPSender(udpSendMock);

    const char* data = "ABC\nDEF\nGHI\n";

    // Process each character to simulate byte-by-byte reception
    for (size_t i = 0; i < strlen(data); i++) {
        handler.processData(&data[i], 1);
    }

    // Should have sent 3 packets (one for each sentence)
    TEST_ASSERT_EQUAL(3, sentPackets.size());
    TEST_ASSERT_EQUAL(4, sentPackets[0].size()); // "ABC\n"
    TEST_ASSERT_EQUAL(4, sentPackets[1].size()); // "DEF\n"
    TEST_ASSERT_EQUAL(4, sentPackets[2].size()); // "GHI\n"
}

// Test: NMEA sentence with CRLF
void test_nmea_with_crlf() {
    handler.reset();
    handler.setUDPSender(udpSendMock);

    const char* sentence = "$GPGGA,123519,4807.038,N\r\n";
    handler.processData(sentence, strlen(sentence));

    TEST_ASSERT_EQUAL(1, sentPackets.size());
    // Should send up to and including '\n'
    TEST_ASSERT_TRUE(sentPackets[0].back() == '\n');
}

// Test: Realistic NMEA stream
void test_realistic_nmea_stream() {
    handler.reset();
    handler.setUDPSender(udpSendMock);

    const char* stream =
        "$GPGGA,123519,4807.038,N,01131.000,E,1,08,0.9,545.4,M,46.9,M,,*47\r\n"
        "$GPGSA,A,3,04,05,,09,12,,,24,,,,,2.5,1.3,2.1*39\r\n"
        "$GPRMC,123519,A,4807.038,N,01131.000,E,022.4,084.4,230394,003.1,W*6A\r\n";

    // Process byte by byte (like real serial reception)
    for (size_t i = 0; i < strlen(stream); i++) {
        handler.processData(&stream[i], 1);
    }

    TEST_ASSERT_EQUAL(3, sentPackets.size());
}

void setUp(void) {
    // Set up before each test
    handler.reset();
}

void tearDown(void) {
    // Clean up after each test
    sentPackets.clear();
}

int main(int argc, char **argv) {
    UNITY_BEGIN();

    RUN_TEST(test_simple_nmea_sentence);
    RUN_TEST(test_multiple_sentences);
    RUN_TEST(test_partial_sentence);
    RUN_TEST(test_partial_then_complete);
    RUN_TEST(test_buffer_overflow_protection);
    RUN_TEST(test_exact_buffer_size);
    RUN_TEST(test_empty_data);
    RUN_TEST(test_only_newline);
    RUN_TEST(test_multiple_newlines);
    RUN_TEST(test_nmea_with_crlf);
    RUN_TEST(test_realistic_nmea_stream);

    return UNITY_END();
}
