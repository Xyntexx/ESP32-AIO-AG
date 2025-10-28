#ifndef MOCK_IMU_H
#define MOCK_IMU_H

#include <functional>

/**
 * Mock implementation of the IMU for heading and roll
 */
class MockIMU {
public:
    // Function types for IMU data
    using GetHeadingFunc = std::function<float()>;
    using GetRollFunc = std::function<float()>;
    
    /**
     * Constructor
     * @param getHeadingFunc Function to call to get heading
     * @param getRollFunc Function to call to get roll
     */
    MockIMU(GetHeadingFunc getHeadingFunc = nullptr, 
           GetRollFunc getRollFunc = nullptr);
    
    /**
     * Get the current heading
     * @return Current heading in degrees
     */
    float getHeading();
    
    /**
     * Get the current roll
     * @return Current roll in degrees
     */
    float getRoll();
    
    /**
     * Set the heading value
     * @param heading Heading in degrees
     */
    void setHeading(float heading);
    
    /**
     * Set the roll value
     * @param roll Roll in degrees
     */
    void setRoll(float roll);
    
private:
    GetHeadingFunc m_getHeadingFunc;
    GetRollFunc m_getRollFunc;
    float m_heading = 0.0f;
    float m_roll = 0.0f;
};

#endif // MOCK_IMU_H 