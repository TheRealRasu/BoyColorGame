#include "Registers.h"

Registers::Registers()
{}

Registers::~Registers()
{}

uint16_t Registers::programCounter() const
{
    return mProgramCounter;
}

uint8_t Registers::accumulator() const
{
    return mAccumulator;
}

uint8_t Registers::singleRegisterValue(const SingleRegister identifier) const
{
    // TODO

    return 0u;
}

uint16_t Registers::combinedRegisterValue(const CombinedRegister identifier) const
{
    switch (identifier)
    {
    case CombinedRegister::register_bc:
    {
        return mBcRegister;
    }
    case CombinedRegister::register_de:
    {
        return mDeRegister;
    }
    case CombinedRegister::register_hl:
    {
        return mHlRegister;
    }
    }

    return 0u;
}

uint8_t& Registers::singleRegister(const uint8_t identifier)
{
    // TODO

    return mInstructionRegister;
}

void Registers::setSingleRegister(const uint8_t identifier, const uint8_t /* value */)
{
    // TODO
}
void Registers::setCombinedRegister(const CombinedRegister identifier, const  uint16_t value)
{
    switch (identifier)
    {
    case CombinedRegister::register_bc:
    {
        mBcRegister = value;
        break;
    }
    case CombinedRegister::register_de:
    {
        mDeRegister = value;
        break;
    }
    case CombinedRegister::register_hl:
    {
        mHlRegister = value;
        break;
    }
    }
}

void Registers::setProgramCounter(const uint16_t newValue)
{
    mProgramCounter = newValue;
}

void Registers::setAccumulator(const uint8_t newValue)
{
    mAccumulator = newValue;
}

void Registers::setInstructionRegister(const uint8_t instruction)
{
    mInstructionRegister = instruction;
}

void Registers::setInterruptEnable(const uint8_t newValue)
{
    mInterruptEnable = newValue;
}