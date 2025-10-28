#include "mock_was.h"

MockWAS::MockWAS(ReadRawFunc readRawFunc)
    : m_readRawFunc(std::move(readRawFunc))
{
}

int16_t MockWAS::readRaw()
{
    // Call the provided function to get the raw value
    return m_readRawFunc();
} 