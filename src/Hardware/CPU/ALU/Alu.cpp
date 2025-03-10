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

template<typename T>
void Alu::incrementRegister(T& givenRegister) const
{
    givenRegister++;
}
template void Alu::incrementRegister<uint8_t>(uint8_t& givenRegister) const;
template void Alu::incrementRegister<uint16_t>(uint16_t& givenRegister) const;

template<typename T>
void Alu::decrementRegister(T& givenValue) const
{
    givenValue--;
}
template void Alu::decrementRegister<uint8_t>(uint8_t& givenValue) const;
template void Alu::decrementRegister<uint16_t>(uint16_t& givenValue) const;

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

void Alu::arithmeticOperation(const uint8_t firstValue, const uint8_t secondValue, const AluOperationType opType, bool additionalFlag)
{
    switch (opType)
    {
        case AluOperationType::add:
        case AluOperationType::add_plus_carry:
        {
            mMemory = firstValue + secondValue + !!additionalFlag;
        }
        case AluOperationType::subtract:
        case AluOperationType::subtract_plus_carry:
        case AluOperationType::compare:
        {
            mMemory = firstValue - secondValue - !!additionalFlag;
        }
        case AluOperationType::logical_and:
        {
            mMemory = firstValue & secondValue;
        }
        case AluOperationType::logical_xor:
        {
            mMemory =  firstValue ^ secondValue;
        }
        case AluOperationType::logical_or:
        {
            mMemory = firstValue | secondValue;
        }
    }

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