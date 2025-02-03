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

uint8_t Registers::smallRegisterValue(const uint8_t identifier) const
{
    switch (identifier)
    {
        case 0b000: return mBRegister;
        case 0b001: return mCRegister;
        case 0b010: return mDRegister;
        case 0b011: return mERegister;
        case 0b100: return mHRegister;
        case 0b101: return mLRegister;
        // 0b110 is the identifier for the data stored in the memory address of the HL value
        case 0b111: return mAccumulator;
        default:
        {
            // do nothing
            break;
        }
    }

    return 0u;
}

uint16_t Registers::bigRegisterValue(const BigRegisterIdentifier identifier) const
{
    switch (identifier)
    {
        case BigRegisterIdentifier::register_bc:
        {
            return (mBRegister << 8) + mCRegister;
        }
        case BigRegisterIdentifier::register_de:
        {
            return (mDRegister << 8) + mERegister;
        }
        case BigRegisterIdentifier::register_hl:
        {
            return (mHRegister << 8) + mLRegister;
        }
        case BigRegisterIdentifier::stack_pointer:
        {
            return mStackPointer;
        }
        case BigRegisterIdentifier::program_counter:
        {
            return mProgramCounter;
        }
    }

    return 0u;
}

void Registers::setSmallRegister(const uint8_t identifier, const uint8_t value)
{
    switch (identifier)
    {
        case 0b000:
        {
            mBRegister = value;
            break;
        }
        case 0b001:
        {
            mCRegister = value;
            break;
        }
        case 0b010:
        {
            mDRegister = value;
            break;
        }
        case 0b011:
        {
            mERegister = value;
            break;
        }
        case 0b100:
        {
            mHRegister = value;
            break;
        }
        case 0b101:
        {
            mLRegister = value;
            break;
        }
        // 0b110 is the identifier for the data stored in the memory address of the HL value
        case 0b111:
        {
            mAccumulator = value;
            break;
        }
        default:
        {
            // do nothing
            break;
        }
    }
}

void Registers::setBigRegister(const BigRegisterIdentifier identifier, const uint16_t value)
{
    switch (identifier)
    {
        case BigRegisterIdentifier::register_bc:
        {
            mBRegister = value >> 8;
            mCRegister = value & 0xFF;
            break;
        }
        case BigRegisterIdentifier::register_de:
        {
            mDRegister = value >> 8;
            mERegister = value & 0xFF;
            break;
        }
        case BigRegisterIdentifier::register_hl:
        {
            mHRegister = value >> 8;
            mLRegister = value & 0xFF;
            break;
        }
        case BigRegisterIdentifier::stack_pointer:
        {
            mStackPointer = value;
            break;
        }
        case BigRegisterIdentifier::program_counter:
        {
            mProgramCounter = value;
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