#pragma once

#include <cstdint>

/*  @ingroup CPU

    class that performs arithmetic operations on given 8- or 16-bit inputs. 
*/

class Alu
{
public:
    Alu() = default;
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

    template<typename T>
    void incrementRegister(T& givenRegister) const;

    template<typename T>
    void decrementRegister(T&  givenRegister) const;

    template<typename T>
    void assignRegisterValue(const T& srcRegister, T& destRegister) const;

    void rotateValue(uint8_t& registerValue, bool& flagValue, const bool rotateRight, const bool throughCarry) const;

    void flipValue(const uint8_t value);

    void arithmeticOperation(const uint8_t firstValue, const uint8_t secondValue, const AluOperationType opType, bool additionalFlag = false);

private:
    uint8_t mMemory {};
};