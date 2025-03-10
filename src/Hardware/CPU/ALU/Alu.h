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

    template<typename T>
    void incrementRegister(T& givenRegister) const;

    template<typename T>
    void decrementRegister(T&  givenRegister) const;

    void loadValueIntoRegister(const uint8_t registerId, const uint8_t value);
    void loadRegisterIntoRegister(const uint8_t destRegister, const uint8_t srcRegister);

    void rotateValue(uint8_t& registerValue, bool& flagValue, const bool rotateRight, const bool throughCarry) const;

    void flipValue(const uint8_t value);

    void arithmeticOperation(const uint8_t firstValue, const uint8_t secondValue, const AluOperationType opType, bool additionalFlag = false);
    void bitOperation(const uint8_t value, uint8_t bitIndex, BitOperationType bitType);

private:
    Registers& mRegisters;

    uint8_t mMemory {};
};