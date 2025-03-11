#include "Registers.h"

Registers::Registers()
{}

Registers::~Registers()
{}

uint16_t Registers::stackPointer() const
{
    return mStackPointer;
}

uint16_t Registers::programCounter() const
{
    return mProgramCounter;
}

uint8_t Registers::accumulator() const
{
    return mAccumulator;
}

uint8_t Registers::instructionRegister() const
{
    return mInstructionRegister;
}

bool Registers::flagValue(FlagsPosition pos) const
{
    return (mFlagsRegister >> static_cast<uint8_t>(pos)) & 0b1;
}

uint8_t Registers::smallRegisterValue(const uint8_t identifier) const
{
    switch (static_cast<Registers::SmallRegisterIdentifier>(identifier))
    {
        case Registers::SmallRegisterIdentifier::register_b: return mBRegister;
        case Registers::SmallRegisterIdentifier::register_c: return mCRegister;
        case Registers::SmallRegisterIdentifier::register_d: return mDRegister;
        case Registers::SmallRegisterIdentifier::register_e: return mERegister;
        case Registers::SmallRegisterIdentifier::register_h: return mHRegister;
        case Registers::SmallRegisterIdentifier::register_l: return mLRegister;
        // 0b110 is the identifier for the data stored in the memory address of the HL value
        case Registers::SmallRegisterIdentifier::register_acc: return mAccumulator;
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
        case BigRegisterIdentifier::register_af:
        {
            return (mAccumulator << 8) + mFlagsRegister;
        }
        case BigRegisterIdentifier::register_sp:
        {
            return mStackPointer;
        }
    }

    return 0u;
}

void Registers::setStackPointer(const uint16_t newValue)
{
    mStackPointer = newValue;
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

void Registers::setFlagValue(FlagsPosition pos, bool value)
{
    const uint8_t flagValue = value << static_cast<uint8_t>(pos);

    if (value)
    {
        mFlagsRegister |= flagValue;
    }
    else
    {
        mFlagsRegister &= ~flagValue;
    }
}

void Registers::setSmallRegister(const uint8_t identifier, const uint8_t value)
{
    switch (static_cast<Registers::SmallRegisterIdentifier>(identifier))
    {
        case Registers::SmallRegisterIdentifier::register_b:
        {
            mBRegister = value;
            break;
        }
        case Registers::SmallRegisterIdentifier::register_c:
        {
            mCRegister = value;
            break;
        }
        case Registers::SmallRegisterIdentifier::register_d:
        {
            mDRegister = value;
            break;
        }
        case Registers::SmallRegisterIdentifier::register_e:
        {
            mERegister = value;
            break;
        }
        case Registers::SmallRegisterIdentifier::register_h:
        {
            mHRegister = value;
            break;
        }
        case Registers::SmallRegisterIdentifier::register_l:
        {
            mLRegister = value;
            break;
        }
        case Registers::SmallRegisterIdentifier::register_acc:
        {
            mAccumulator = value;
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
        case BigRegisterIdentifier::register_af:
        {
            mAccumulator = value >> 8;
            mFlagsRegister = value & 0xFF;
            break;
        }
        case BigRegisterIdentifier::register_sp:
        {
            mStackPointer = value;
            break;
        }
    }
}

bool Registers::checkFlagCondition(FlagCondition condition) const
{
    switch (condition)
    {
        case FlagCondition::condition_nz:
        {
            return flagValue(FlagsPosition::zero_flag) == false;
        }
        case FlagCondition::condition_z:
        {
            return flagValue(FlagsPosition::zero_flag);
        }
        case FlagCondition::condition_nc:
        {
            return flagValue(FlagsPosition::carry_flag) == false;
        }
        case FlagCondition::condition_c:
        {
            return flagValue(FlagsPosition::carry_flag);
        }
    }

    return false;
}

Registers::BigRegisterIdentifier Registers::instructionToBigRegisterId(const uint8_t instructionCode)
{
    switch (instructionCode >> 4 & 0b11)
    {
        case 0b00: return Registers::BigRegisterIdentifier::register_bc;
        case 0b01: return Registers::BigRegisterIdentifier::register_de;
        case 0b10: return Registers::BigRegisterIdentifier::register_hl;
        case 0b11: // 0b11 either returns AF, HL, or SP, depending on other instruction bits
        {
            if (((instructionCode >> 6) & 0b11) == 0b11)
            {
                return Registers::BigRegisterIdentifier::register_af;
            }

            if ((instructionCode & 0b11) == 0b10)
            {
                return Registers::BigRegisterIdentifier::register_hl;
            }           

            return Registers::BigRegisterIdentifier::register_sp;
        }
        default: break;
    }

    return Registers::BigRegisterIdentifier::register_bc;
}