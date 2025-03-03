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

    void increaseValue(const uint16_t givenValue);

    void decreaseValue(const uint16_t givenValue);

    void assignValue(const uint16_t value);

private:
    uint16_t mMemory {};
};