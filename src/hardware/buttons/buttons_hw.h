#ifndef BUTTONS_H
#define BUTTONS_H

namespace hw {

// Buttons hardware implementation
class Buttons {
public:
    static bool init();
    static bool steerPinState();
    static void handler();

private:
    static bool initialized;
};

} // namespace hw

#endif // BUTTONS_H 