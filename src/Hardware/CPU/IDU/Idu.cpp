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

void Idu::increaseRegister(Registers::BigRegisterIdentifier bigRegister, const uint16_t value)
{
    mRegisters.setBigRegister(bigRegister, value + 1);
}

void Idu::decreaseRegister(Registers::BigRegisterIdentifier bigRegister, const uint16_t value)
{
    mRegisters.setBigRegister(bigRegister, value - 1);
}

void Idu::incrementProgramCounter()
{
    const uint16_t programCounter = mRegisters.programCounter();
    mRegisters.setProgramCounter(programCounter + 1);
}

void Idu::decrementProgramCounter()
{
    const uint16_t programCounter = mRegisters.programCounter();
    mRegisters.setProgramCounter(programCounter - 1);
}

void Idu::incrementStackPointer()
{
    const uint16_t stackPointer = mRegisters.stackPointer();
    mRegisters.setStackPointer(stackPointer + 1);
}

void Idu::decrementStackPointer()
{
    const uint16_t stackPointer = mRegisters.stackPointer();
    mRegisters.setStackPointer(stackPointer - 1);
}