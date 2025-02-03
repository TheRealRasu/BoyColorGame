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

    enum class ArithmeticOperationType
    {
        add = 0,
        add_plus_carry = 1,
        compare = 2,
        complement_accumulator = 3,
        complement_carry_flag = 4,
        decimal_adjust_accumulator = 5,
        increment = 6,
        logical_and = 7,
        logical_or = 8,
        logical_xor = 9,
        set_carry_flag = 10,
        subtract = 11,
        subtract_plus_carry = 12,
    };

    template<typename T>
    void incrementRegister(T& givenRegister) const;

    template<typename T>
    void decrementRegister(T& givenRegister) const;

    template<typename T>
    void assignRegisterValue(const T& srcRegister, T& destRegister) const;

    uint8_t arithmeticOperation(const uint8_t firstValue, const uint8_t secondValue, const uint8_t arithmeticOpType) const;

    void flipRegister(uint8_t& value);
};