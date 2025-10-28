#ifndef MOCK_WAS_H
#define MOCK_WAS_H

#include <functional>
#include <cstdint>

/**
 * Mock implementation of the WAS (Wheel Angle Sensor)
 */
class MockWAS {
public:
    // Function type for raw reading callback
    using ReadRawFunc = std::function<int16_t()>;
    
    /**
     * Constructor
     * @param readRawFunc Function to call when the WAS is read
     */
    explicit MockWAS(ReadRawFunc readRawFunc);
    
    /**
     * Read raw WAS value
     * @return Raw WAS value
     */
    int16_t readRaw();
    
private:
    ReadRawFunc m_readRawFunc;
};

#endif // MOCK_WAS_H 