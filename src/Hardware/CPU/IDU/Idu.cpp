#include "Idu.h"

Idu::Idu(Registers& registers)
    : mRegisters(registers)
{}

uint16_t Idu::memory() const
{
    return mMemory;
}

void Idu::resetMemory()
{
    mMemory = 0u;
}

void Idu::increaseValue(const uint16_t givenValue)
{
    mMemory = givenValue + 1;
}

void Idu::decreaseValue(const uint16_t givenValue)
{
    mMemory = givenValue -1 ;
}

void Idu::assignValue(const uint16_t value)
{
    mMemory = value;
}