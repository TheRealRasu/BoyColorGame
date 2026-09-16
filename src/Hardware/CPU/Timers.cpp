#include "Timers.h"

#include <limits>

Timers::Timers()
{

}

Timers::~Timers()
{

}

bool Timers::timerOverflow() const
{
    return mTickCycles == 0;
}

bool Timers::counterOverflow() const
{
    return mCounterOverflow;
}

bool Timers::newFrame() const
{
    // return
}

void Timers::startTimer()
{
    mControl |= 0b100;
}

void Timers::stopTimer()
{
    mControl &= 0b11;
}

void Timers::setTimerSpeed(uint8_t speedBits)
{
    mControl |= speedBits;
}

void Timers::update()
{
    mTickCycles += 8;
    if (mTickCycles == 0) mDivider++;
    
    mMachineCycles += 2;

    if (mMachineCycles % 4 == 0) mBaseCycles++;

    const bool counterMax = (mCounter == std::numeric_limits<uint8_t>::max());

    switch (mControl & 0b11)
    {
        case 0b00:
        {
            if (mMachineCycles == 0) mCounter++;
            break;
        }
        case 0b01:
        {
            if ((mMachineCycles % 4) == 0) mCounter++;
            break;
        }
        case 0b10:
        {
            if ((mMachineCycles % 16) == 0) mCounter++;
            break;
        }
        case 0b11:
        {
            if ((mMachineCycles % 64) == 0) mCounter++;
            break;
        }
        default: break;
    }

    mCounterOverflow = (counterMax && mCounter == 0);
    if (mCounterOverflow) mCounter = mModulo;
}