#ifndef MOCK_BUTTONS_H
#define MOCK_BUTTONS_H

#include <functional>
#include <cstdint>

/**
 * Mock implementation of the steering buttons
 */
class MockButtons {
public:
    // Function type for reading button state
    using ReadFunc = std::function<bool()>;
    
    /**
     * Constructor
     * @param readFunc Function to call when button state is read
     */
    explicit MockButtons(ReadFunc readFunc);
    
    /**
     * Read steering button state
     * @return Button state (true = pressed/enabled, false = released/disabled)
     */
    bool steerPinState();
    
private:
    ReadFunc m_readFunc;
};

#endif // MOCK_BUTTONS_H 