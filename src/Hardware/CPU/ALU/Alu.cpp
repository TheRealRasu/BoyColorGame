#include "Alu.h"

template<typename T>
T Alu::incrementRegister(const T givenRegister) const
{
    return givenRegister + 1;
}
template uint8_t Alu::incrementRegister<uint8_t>(const uint8_t givenRegister) const;
template uint16_t Alu::incrementRegister<uint16_t>(const uint16_t givenRegister) const;

template<typename T>
T Alu::decrementRegister(const T givenValue) const
{
    return givenValue - 1;
}
template uint8_t Alu::decrementRegister<uint8_t>(const uint8_t givenValue) const;
template uint16_t Alu::decrementRegister<uint16_t>(const uint16_t givenValue) const;

template<typename T>
void Alu::assignRegisterValue(const T& srcRegister, T& destRegister) const
{
    destRegister = srcRegister;
}
template void Alu::assignRegisterValue<uint8_t>(const uint8_t& srcRegister, uint8_t& destRegister) const;
template void Alu::assignRegisterValue<uint16_t>(const uint16_t& srcRegister, uint16_t& destRegister) const;

void rotateValue(uint8_t& registerValue, bool& flagValue, const bool rotateRight, const bool throughCarry) const
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

return;
}

uint8_t Alu::flipValue(const uint8_t value) const
{
    return ~value;
}

uint8_t Alu::arithmeticOperation(const uint8_t firstValue, const uint8_t secondValue, const AluOperationType opType, bool additionalFlag) const
{
    switch (opType)
    {
        case AluOperationType::add:
        case AluOperationType::add_plus_carry:
        {
            return firstValue + secondValue + !!additionalFlag;
        }
        case AluOperationType::subtract:
        case AluOperationType::subtract_plus_carry:
        case AluOperationType::compare:
        {
            return firstValue - secondValue - !!additionalFlag;
        }
        case AluOperationType::logical_and:
        {
            return firstValue & secondValue;
        }
        case AluOperationType::logical_xor:
        {
            return firstValue ^ secondValue;
        }
        case AluOperationType::logical_or:
        {
            return firstValue | secondValue;
        }
    }

    return 0u;
}
