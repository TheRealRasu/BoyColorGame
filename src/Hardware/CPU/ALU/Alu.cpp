#include "Alu.h"

#include <bitset>

Alu::Alu(Registers& registers)
    : mRegisters(registers)
{}

uint8_t Alu::memory() const
{
    return mMemory;
}

void Alu::resetMemory()
{
    mMemory = 0u;
}

void Alu::incrementRegister(const uint8_t givenRegister)
{
    mMemory = givenRegister + 1;
}

void Alu::decrementRegister(const uint8_t givenValue)
{
    mMemory = givenValue - 1;
}

void Alu::loadValueIntoRegister(const uint8_t registerId, const uint8_t value)
{
    mRegisters.setSmallRegister(registerId, value);
}

void Alu::loadRegisterIntoRegister(const uint8_t destRegister, const uint8_t srcRegister)
{
    const uint8_t newValue = mRegisters.smallRegisterValue(srcRegister);
    mRegisters.setSmallRegister(destRegister, newValue);
}

void Alu::rotateValue(uint8_t& registerValue, bool& flagValue, const bool rotateRight, const bool throughCarry) const
{
    // const uint8_t rightShift = rotateRight ? 0u : 7u;
    // const uint8_t leftShift = 7u - rightShift;
    
    // const bool newFlag = (registerValue >> rightShift) & 0b1;

    // if (rotateRight)
    // {
    //     registerValue >>= 1;
    // }
    // else
    // {
    //     registerValue <<= 1;
    // }

    // const bool& additionalFlag = throughCarry ? flagValue : newFlag;

    // registerValue += additionalFlag << leftShift;

    if (rotateRight)
    {
        const bool newFlag = registerValue & 0b1;
        registerValue >>= 1;

        if (throughCarry)
        {
            registerValue += (flagValue << 7);
        }
        else
        {
            registerValue += (newFlag << 7);
        }

        flagValue = newFlag;
    }
    else
    {
        const bool newFlag = (registerValue >> 7) & 0b1;
        registerValue <<= 1;
                
        if (throughCarry)
        {
            registerValue += flagValue;
        }
        else
        {
            registerValue += newFlag;
        }
        
        flagValue = newFlag;
    }
}

void Alu::flipValue(const uint8_t value)
{
    mMemory = ~value;
}

void Alu::arithmeticOperation(const uint8_t otherValue, const AluOperationType opType)
{
    const uint8_t accumulatorValue = mRegisters.accumulator();
    uint8_t additionalValue = 0;

    switch (opType)
    {
        case AluOperationType::add_plus_carry:
        {
            additionalValue = !!mRegisters.flagValue(Registers::FlagsPosition::carry_flag);
            [[fallthrough]];
        }
        case AluOperationType::add:
        {
            mMemory = accumulatorValue + otherValue + additionalValue;
            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (((mMemory >> 3) & 0b1) == 0b1));
            mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, (((mMemory >> 7) & 0b1) == 0b1));
            break;
        }
        case AluOperationType::subtract_plus_carry:
        {
            additionalValue = !!mRegisters.flagValue(Registers::FlagsPosition::carry_flag);
            [[fallthrough]];
        }
        case AluOperationType::subtract:
        case AluOperationType::compare:
        {
            mMemory = accumulatorValue - otherValue - additionalValue;

            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, true);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (((mMemory >> 3) & 0b1) == 0b1));
            mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, (((mMemory >> 7) & 0b1) == 0b1));
            break;
        }
        case AluOperationType::logical_and:
        {
            mMemory = accumulatorValue & otherValue;

            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, true);
            mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, false);
            break;
        }
        case AluOperationType::logical_xor:
        {
            mMemory =  accumulatorValue ^ otherValue;

            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, false);
            break;
        }
        case AluOperationType::logical_or:
        {
            mMemory = accumulatorValue | otherValue;

            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, false);
            break;
        }
    }

    mRegisters.setFlagValue(Registers::FlagsPosition::zero_flag, (mMemory == 0));

    if (opType != Alu::AluOperationType::compare)
    {
        mRegisters.setAccumulator(mMemory);
    }
}

void Alu::bitOperation(const uint8_t value, uint8_t bitIndex, BitOperationType bitType)
{
    std::bitset<8> bits = std::bitset<8>(value);
    switch (bitType)
    {
        case BitOperationType::swap_nibbles:
        {
            // TODO
            break;
        }
        case BitOperationType::test_bit:
        {
            const bool bitTest = bits.test(bitIndex);

            mMemory = !!bitTest;
            break;
    
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, true);
            mRegisters.setFlagValue(Registers::FlagsPosition::zero_flag, bitTest == false);
            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
        }
        case BitOperationType::reset_bit:
        {
            bits.reset(bitIndex);
            mMemory = static_cast<uint8_t>(bits.to_ulong());
            break;
        }
        case BitOperationType::set_bit:
        {
            bits.set(bitIndex);
            mMemory = static_cast<uint8_t>(bits.to_ulong());

            break;
        }
    }

}