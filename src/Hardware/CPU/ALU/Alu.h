#pragma once

#include "../Registers/Registers.h"

#include <cstdint>

/*  @ingroup CPU

    class that performs arithmetic operations on given 8- or 16-bit inputs. 
*/

class Alu
{
public:
    Alu(Registers& registers);
    ~Alu() = default;

    uint8_t memory() const;
    void resetMemory();

    enum class AluOperationType : uint8_t
    {
        add = 0b000,
        add_plus_carry = 0b001,
        subtract = 0b010,
        subtract_plus_carry = 0b011,
        logical_and = 0b100,
        logical_xor = 0b101,
        logical_or = 0b110,
        compare = 0b111
    };

    enum class BitOperationType : uint8_t
    {
        swap_nibbles = 0b00,
        test_bit = 0b01,
        reset_bit = 0b10,
        set_bit = 0b11
    };

    void incrementRegister(const uint8_t givenRegister);
    void decrementRegister(const uint8_t givenRegister);

    void incrementValue(const uint8_t givenValue);
    void decrementValue(const uint8_t givenValue);

    void addToRegister(const uint8_t registerId, const uint8_t value);

    void loadValueIntoRegister(const uint8_t registerId, const uint8_t value);
    void loadRegisterIntoRegister(const uint8_t destRegister, const uint8_t srcRegister);

    void rotateLeft(const uint8_t registerId, const bool circular);
    void rotateRight(const uint8_t registerId, const bool circular);

    void flipRegister(const uint8_t registerId);

    void arithmeticAccumulatorOperation(const uint8_t otherValue, const AluOperationType opType);
    void bitOperation(const uint8_t value, uint8_t bitIndex, BitOperationType bitType);

    void setCarryFlag();
    void complementCarryFlag();

    void decimalAdjustAccumulator();

private:
    void setFlagsAfterOperation(const AluOperationType opType, const bool includeCarryFlag, const bool includeZeroFlag);

    Registers& mRegisters;

    uint8_t mMemory {};
};