#include "Registers.h"

Registers::Registers()
{}

Registers::~Registers()
{}

uint16_t Registers::programCounter() const
{
    return mProgramCounter;
}

uint8_t Registers::singleRegisterValue(uint8_t identifier) const
{
    // TODO

    return 0u;
}

uint16_t Registers::combinedRegisterValue(uint8_t identifier) const
{
    // TODO

    return 0u;
}

uint8_t& Registers::singleRegister(uint8_t identifier)
{
    // TODO

    return mBRegister;
}

void setSingleRegister(uint8_t identifier, uint8_t /* value */)
{
    // TODO
}
void setCombinedRegister(uint8_t identifier, uint16_t /* value */)
{
    // TODO
}

void Registers::setProgramCounter(uint16_t newValue)
{
    mProgramCounter = newValue;
}

void Registers::setInstructionRegister(uint8_t instruction)
{
    mInstructionRegister = instruction;
}