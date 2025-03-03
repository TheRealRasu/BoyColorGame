#pragma once

#include <cstdint>

/*  @ingroup CPU

    class that exclusive alters given variables by incrementation, decrementation, or by copying a given value 
*/
class Idu
{
public:
    Idu() = default;
    ~Idu() = default;

    uint16_t memory() const;
    void resetMemory();

    void increaseValue(uint16_t& givenValue) const;

    template<typename T>
    void decreaseValue(T& givenValue) const;

    template<typename T>
    void assignValue(const T& valueSrc, T& valueDst) const;

private:
    uint16_t mMemory {};
};