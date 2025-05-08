#ifndef BUTTONS_H
#define BUTTONS_H

namespace hw {
// Buttons hardware implementation
class Buttons {
    public:
        static bool init();
        static bool steerPinState();
        static bool workPinState();
        static void handler();

    private:
        static bool initialized_;
};
} // namespace hw

#endif // BUTTONS_H
