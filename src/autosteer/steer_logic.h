#ifndef STEER_LOGIC_H
#define STEER_LOGIC_H

namespace autosteer {
    // Steering switch types
    enum class steer_switch_type_types : uint8_t {
        SWITCH = 0,         // Physical on/off switch
        momentary = 1,      // Physical momentary button
        none = 2            // No physical button
    };

    // Guidance packet validation
    bool guidancePacketValid();

    // Software switch status
    bool getSwSwitchStatus();
}

#endif // STEER_LOGIC_H 