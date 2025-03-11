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
    mMemory = mRegisters.smallRegisterValue(givenRegister) + 1;
    mRegisters.setSmallRegister(givenRegister, mMemory);

    setFlagsAfterOperation(Alu::AluOperationType::add, false);
}

void Alu::decrementRegister(const uint8_t givenRegister)
{
    mMemory = mRegisters.smallRegisterValue(givenRegister) - 1;
    mRegisters.setSmallRegister(givenRegister, mMemory);

    setFlagsAfterOperation(Alu::AluOperationType::subtract, false);
}

void Alu::incrementValue(const uint8_t givenValue)
{
    mMemory = givenValue + 1;
    setFlagsAfterOperation(Alu::AluOperationType::add, false);
}

void Alu::decrementValue(const uint8_t givenValue)
{
    mMemory = givenValue - 1;
    setFlagsAfterOperation(Alu::AluOperationType::subtract, false);
}

void Alu::addToRegister(const uint8_t registerId, const uint8_t value)
{
    mMemory = mRegisters.smallRegisterValue(registerId) + value;
    mRegisters.setSmallRegister(registerId, mMemory);

    setFlagsAfterOperation(Alu::AluOperationType::add, true, false);
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

void Alu::rotateLeft(const uint8_t registerId, const bool circular)
{
    mMemory = mRegisters.smallRegisterValue(registerId);

    const bool highestBit = (mMemory >> 7) & 0b1;
    const bool shiftedBit = circular ? highestBit : mRegisters.flagValue(Registers::FlagsPosition::carry_flag);

    mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, highestBit);

    mMemory <<= 1;
    mMemory += shiftedBit;

    mRegisters.setSmallRegister(registerId, mMemory);

    // TODO set flags
    // TODO check if these flag assignments are accurate
    mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, false);
    mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
}

void Alu::rotateRight(const uint8_t registerId, const bool circular)
{
    mMemory = mRegisters.smallRegisterValue(registerId);

    const bool lowestBit = mMemory & 0b1;
    const bool shiftedBit = circular ? lowestBit : mRegisters.flagValue(Registers::FlagsPosition::carry_flag);

    mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, lowestBit);

    mMemory >>= 1;
    mMemory += (shiftedBit << 7);

    mRegisters.setSmallRegister(registerId, mMemory);

    // TODO set flags
    // TODO check if these flag assignments are accurate
    mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, false);
    mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
}

void Alu::flipRegister(const uint8_t registerId)
{
    mMemory = ~mRegisters.smallRegisterValue(registerId);
    mRegisters.setSmallRegister(registerId, mMemory);
    
    mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, true);
    mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, true);
}

void Alu::arithmeticAccumulatorOperation(const uint8_t otherValue, const AluOperationType opType)
{
    mMemory = mRegisters.accumulator();

    switch (opType)
    {
        case AluOperationType::add_plus_carry:
        case AluOperationType::add:
        {
            mMemory += otherValue;
            if (opType == AluOperationType::add_plus_carry)
            {
                mMemory += !!mRegisters.flagValue(Registers::FlagsPosition::carry_flag);
            }
            break;
        }
        case AluOperationType::subtract_plus_carry:
        case AluOperationType::subtract:
        case AluOperationType::compare:
        {
            mMemory -= otherValue;
            if (opType == AluOperationType::subtract_plus_carry)
            {
                mMemory -= !!mRegisters.flagValue(Registers::FlagsPosition::carry_flag);
            }
            break;
        }
        case AluOperationType::logical_and:
        {
            mMemory &= otherValue;
            break;
        }
        case AluOperationType::logical_xor:
        {
            mMemory ^=  otherValue;
            break;
        }
        case AluOperationType::logical_or:
        {
            mMemory |= otherValue;
            break;
        }
    }

    setFlagsAfterOperation(opType, true);

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

void Alu::setCarryFlag()
{
    mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
    mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, false);
    mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, true);
}

void Alu::complementCarryFlag()
{
    mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
    mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, false);

    const bool carry = mRegisters.flagValue(Registers::FlagsPosition::carry_flag);
    mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, carry == false);
}

void Alu::decimalAdjustAccumulator()
{
    mMemory = mRegisters.accumulator();

    // TODO. Consult with CPU documents
}

void Alu::setFlagsAfterOperation(const AluOperationType opType, const bool includeCarryFlag, const bool includeZeroFlag)
{
    switch (opType)
    {
        case AluOperationType::add_plus_carry:
        case AluOperationType::add:
        {
            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (((mMemory >> 3) & 0b1) == 0b1));
            if (includeCarryFlag) mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, (((mMemory >> 7) & 0b1) == 0b1));

            break;
        }
        case AluOperationType::subtract_plus_carry:
        case AluOperationType::subtract:
        case AluOperationType::compare:
        {
            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, true);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (((mMemory >> 3) & 0b1) == 0b1));
            if (includeCarryFlag) mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, (((mMemory >> 7) & 0b1) == 0b1));

            break;
        }
        case AluOperationType::logical_and:
        case AluOperationType::logical_xor:
        case AluOperationType::logical_or:
        {
            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, opType == AluOperationType::logical_and);
            if (includeCarryFlag) mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, false);

            break;
        }
    }

    if (includeZeroFlag) mRegisters.setFlagValue(Registers::FlagsPosition::zero_flag, (mMemory == 0));
}