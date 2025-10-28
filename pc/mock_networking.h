#ifndef MOCK_NETWORKING_H
#define MOCK_NETWORKING_H

#include <functional>
#include <cstdint>

/**
 * Mock implementation of the UDP/networking communication
 */
class MockNetworking {
public:
    // Function types for communication callbacks
    using GetSteerSetpointFunc = std::function<float()>;
    using GetSwSwitchStatusFunc = std::function<bool()>;
    
    /**
     * Constructor
     * @param getSteerSetpointFunc Function to call to get steering setpoint
     * @param getSwSwitchStatusFunc Function to call to get software switch status
     */
    MockNetworking(GetSteerSetpointFunc getSteerSetpointFunc, 
                  GetSwSwitchStatusFunc getSwSwitchStatusFunc);
    
    /**
     * Get the current steering setpoint
     * @return Current steering setpoint in degrees
     */
    float getSteerSetpoint();
    
    /**
     * Get the current software switch status
     * @return Current software switch status (true = enabled, false = disabled)
     */
    bool getSwSwitchStatus();
    
private:
    GetSteerSetpointFunc m_getSteerSetpointFunc;
    GetSwSwitchStatusFunc m_getSwSwitchStatusFunc;
};

// Function to hook into the UDP functionality
void setupMockUdpFunctions(MockNetworking* mock);

#endif // MOCK_NETWORKING_H 