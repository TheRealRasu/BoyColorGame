#pragma once

#include "../Registers/Registers.h"

#include <cstdint>

/*  @ingroup CPU

    class that exclusive alters given variables by incrementation, decrementation, or by copying a given value 
*/

class Idu
{
public:
    Idu(Registers& registers);
    ~Idu() = default;

    uint16_t memory() const;
    void resetMemory();

    // TODO: give big register identifiers
    void increaseValue(const uint16_t givenValue);

    void decreaseValue(const uint16_t givenValue);

    void assignValue(const uint16_t value);

    void assignAndIncrementRegister(Registers::BigRegisterIdentifier bigRegister, const uint16_t value);
    void assignAndDecrementRegister(Registers::BigRegisterIdentifier bigRegister, const uint16_t value);

    void incrementRegister(Registers::BigRegisterIdentifier registerId);
    void decrementRegister(Registers::BigRegisterIdentifier registerId);

    void incrementProgramCounter();
    void decrementProgramCounter();

    void incrementStackPointer();
    void decrementStackPointer();

private:
    Registers& mRegisters;

    uint16_t mMemory {};
};