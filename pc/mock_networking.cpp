#include "mock_networking.h"
#include "src/autosteer/udp_io.h"

// Global pointer to the active mock
static MockNetworking* g_activeNetworkingMock = nullptr;

// Forward declaration of the hook functions
extern "C" {
    float __mock_getSteerSetPoint();
    bool __mock_getSwSwitchStatus();
    bool __mock_guidancePacketValid();
}

MockNetworking::MockNetworking(GetSteerSetpointFunc getSteerSetpointFunc, 
                             GetSwSwitchStatusFunc getSwSwitchStatusFunc)
    : m_getSteerSetpointFunc(std::move(getSteerSetpointFunc))
    , m_getSwSwitchStatusFunc(std::move(getSwSwitchStatusFunc))
{
    // Register as the active mock
    g_activeNetworkingMock = this;
    
    // Hook the UDP functions
    setupMockUdpFunctions(this);
}

float MockNetworking::getSteerSetpoint()
{
    if (m_getSteerSetpointFunc) {
        return m_getSteerSetpointFunc();
    }
    return 0.0f;
}

bool MockNetworking::getSwSwitchStatus()
{
    if (m_getSwSwitchStatusFunc) {
        return m_getSwSwitchStatusFunc();
    }
    return false;
}

// Hook functions that override the real UDP implementation
float __mock_getSteerSetPoint()
{
    if (g_activeNetworkingMock) {
        return g_activeNetworkingMock->getSteerSetpoint();
    }
    return 0.0f;
}

bool __mock_getSwSwitchStatus()
{
    if (g_activeNetworkingMock) {
        return g_activeNetworkingMock->getSwSwitchStatus();
    }
    return false;
}

bool __mock_guidancePacketValid()
{
    // Always return true for simulation
    return true;
}

// Hook the UDP functions to use our mock implementation
void setupMockUdpFunctions(MockNetworking* mock)
{
    // These hooks will be used by the real autosteer code
    // Note: This approach requires modifying the udp_io.cpp to use these functions
    // if in PC simulation mode. Alternatively, we'd need to create a proper mock
    // with the same function signatures.
    
    // For simplicity, we'll assume the following functions are defined in udp_io.cpp
    // and handle the PC simulation case:
    //
    // float getSteerSetPoint() {
    //     #ifdef PC_SIMULATION
    //         return __mock_getSteerSetPoint();
    //     #else
    //         return real implementation...
    //     #endif
    // }
} 