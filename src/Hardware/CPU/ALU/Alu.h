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

    template<typename T>
    void incrementRegister(T& givenRegister) const;

    template<typename T>
    void decrementRegister(T& givenRegister) const;

    template<typename T>
    void assignRegisterValue(const T& srcRegister, T& destRegister) const;
};