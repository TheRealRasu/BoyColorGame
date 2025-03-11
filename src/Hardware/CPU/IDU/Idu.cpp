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

void Idu::assignAndIncrementRegister(Registers::BigRegisterIdentifier bigRegister, const uint16_t value)
{
    mMemory = value + 1;
    mRegisters.setBigRegister(bigRegister, mMemory);
}

void Idu::assignAndDecrementRegister(Registers::BigRegisterIdentifier bigRegister, const uint16_t value)
{
    mMemory = value - 1;
    mRegisters.setBigRegister(bigRegister, mMemory);
}

void Idu::incrementRegister(Registers::BigRegisterIdentifier registerId)
{
    mMemory = mRegisters.bigRegisterValue(registerId) + 1;
    mRegisters.setBigRegister(registerId, mMemory);
}

void Idu::decrementRegister(Registers::BigRegisterIdentifier registerId)
{
    mMemory = mRegisters.bigRegisterValue(registerId) - 1;
    mRegisters.setBigRegister(registerId, mMemory);
}

void Idu::incrementProgramCounter()
{
    mMemory = mRegisters.programCounter() + 1;
    mRegisters.setProgramCounter(mMemory);
}

void Idu::decrementProgramCounter()
{
    mMemory = mRegisters.programCounter() - 1;
    mRegisters.setProgramCounter(mMemory);
}

void Idu::incrementStackPointer()
{
    mMemory = mRegisters.stackPointer() + 1;
    mRegisters.setStackPointer(mMemory);
}

void Idu::decrementStackPointer()
{
    mMemory = mRegisters.stackPointer() - 1;
    mRegisters.setStackPointer(mMemory);
}