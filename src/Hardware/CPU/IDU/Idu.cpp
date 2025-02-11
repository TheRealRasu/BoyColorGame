#include "Idu.h"

template<typename T>
void Idu::increaseValue(T& givenValue) const
{
    givenValue++;
}
template void Idu::increaseValue<uint8_t>(uint8_t& givenValue) const;
template void Idu::increaseValue<uint16_t>(uint16_t& givenValue) const;

template<typename T>
void Idu::decreaseValue(T& givenValue) const
{
    givenValue--;
}
template void Idu::decreaseValue<uint8_t>(uint8_t& givenValue) const;
template void Idu::decreaseValue<uint16_t>(uint16_t& givenValue) const;

template<typename T> 
void Idu::assignValue(const T& valueSrc, T& valueDst) const
{
    valueDst = valueSrc;
}
template void Idu::assignValue<uint8_t>(const uint8_t& valueSrc, uint8_t& valueDst) const;
template void Idu::assignValue<uint16_t>(const uint16_t& valueSrc, uint16_t& valueDst) const;