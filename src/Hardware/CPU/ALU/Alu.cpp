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

uint8_t Alu::flipValue(const uint8_t value)
{
    return ~value;
}

uint8_t Alu::arithmeticOperation(const uint8_t firstValue, const uint8_t secondValue, const uint8_t arithmeticOpType) const
{
    switch (arithmeticOpType)
    {
        case 0b000: // add
        {
            return firstValue + secondValue;
        }
        case 0b001: //adc
        {
            // TODO
            break;
        }
        case 0b010: // sub
        {
            return firstValue - secondValue;
        }
        case 0b011: // sbc
        {
            // TODO
            break;
        }
        case 0b100: // and
        {
            return firstValue & secondValue;
        }
        case 0b101: // xor
        {
            return firstValue ^ secondValue;
        }
        case 0b110: // or
        {
            return firstValue | secondValue;
        }
        case 0b111: //cp
        {
            // TODO
            break;
        }
        default:
        {
            // do nothing
            break;
        }
    }

    return 0u;
}

/*


        case ArithmeticOperationType::set_carry_flag:
        {
            // TODO
            break;
        }
        case ArithmeticOperationType::complement_accumulator:
        {
            // TODO
            break;
        }
        case ArithmeticOperationType::complement_carry_flag:
        {
            // TODO
            break;
        }
        case ArithmeticOperationType::decimal_adjust_accumulator:
        {
            // TODO
            break;
        }
        case ArithmeticOperationType::increment:
        {
            // TODO
            break;
        }




*/