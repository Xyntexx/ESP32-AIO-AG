#include "mock_buttons.h"

MockButtons::MockButtons(ReadFunc readFunc)
    : m_readFunc(std::move(readFunc))
{
}

bool MockButtons::steerPinState()
{
    // Call the provided function to get the button state
    if (m_readFunc) {
        return m_readFunc();
    }
    return false;
} 