#include "Alu.h"

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

template<typename T> 
void Alu::assignRegisterValue(const T& srcRegister, T& destRegister) const
{
    destRegister = srcRegister;
}
template void Alu::assignRegisterValue<uint8_t>(const uint8_t& srcRegister, uint8_t& destRegister) const;
template void Alu::assignRegisterValue<uint16_t>(const uint16_t& srcRegister, uint16_t& destRegister) const;

void Alu::flipRegister(uint8_t& value)
{
    value = ~value;
}