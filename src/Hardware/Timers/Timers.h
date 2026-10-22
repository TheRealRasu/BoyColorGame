#pragma once

#include <cstdint>

class Timers
{
public:
    Timers();
    ~Timers() = default;

    void update();
    bool timerOverflow() const;
    bool counterOverflow() const;
    bool newFrame() const;

    void startTimer();
    void stopTimer();

    void setTimerSpeed(uint8_t speedBits);

protected:
    uint8_t mTickCycles {}; // incremented at 2^22 Hz.
    uint8_t mMachineCycles {}; // incremented at 2^20 Hz.
    uint8_t mBaseCycles {}; // incremented at 2^18 Hz.

    uint8_t mDivider {}; // incremented at 2^14 Hz.
    uint8_t mCounter {}; // incremented at custom speed.
    uint8_t mModulo {}; // incremented at 2^14 Hz.
    uint8_t mControl {}; // Bits 0 and 1 document the Counter speed. Bit 2: Start/Stop. Bits 3-7 unused.

    bool mCounterOverflow {};
};