#include "mock_imu.h"

MockIMU::MockIMU(GetHeadingFunc getHeadingFunc, GetRollFunc getRollFunc)
    : m_getHeadingFunc(std::move(getHeadingFunc))
    , m_getRollFunc(std::move(getRollFunc))
{
}

float MockIMU::getHeading()
{
    if (m_getHeadingFunc) {
        return m_getHeadingFunc();
    }
    return m_heading;
}

float MockIMU::getRoll()
{
    if (m_getRollFunc) {
        return m_getRollFunc();
    }
    return m_roll;
}

void MockIMU::setHeading(float heading)
{
    m_heading = heading;
}

void MockIMU::setRoll(float roll)
{
    m_roll = roll;
} 