#pragma once

#include <cstdint> 

class Registers
{
public:
    Registers();
    ~Registers();

    enum class SingleRegister : uint8_t
    {
        register_b = 0b000,

    };

    enum class CombinedRegister : uint8_t
    {
        register_bc = 0,
        register_de = 1,
        register_hl = 2
    };
    
    // getters
    uint16_t programCounter() const;
    uint8_t accumulator() const;

    uint8_t singleRegisterValue(const SingleRegister identifier) const;
    uint16_t combinedRegisterValue(const CombinedRegister identifier) const;

    uint8_t& singleRegister(const uint8_t identifier);

    // setters
    void setSingleRegister(const uint8_t identifier, const uint8_t value);
    void setCombinedRegister(const CombinedRegister identifier, const uint16_t value);


    void setProgramCounter(const uint16_t newValue);
    void setAccumulator(const uint8_t newValue);
    void setInstructionRegister(const uint8_t instruction);
    void setInterruptEnable(const uint8_t newValue);


    /* TODO assign this to registers

    uint16_t bc = 0x1234;
    
    uint8_t* b = reinterpret_cast<uint8_t*>(&bc) + sizeof(uint8_t);
    uint8_t* c = reinterpret_cast<uint8_t*>(&bc);
    
    std::cout << "&bc=" << &bc << "\n&b=" << &b << "\n&c=" << &c << "\n";
    std::cout << "bc=" << std::to_string(bc) << ",b=" << std::to_string(*b) << ",c=" << std::to_string(*c) << "\n";

    (*b)++;
    (*c) += 3;
    std::cout << "&bc=" << &bc << "\n&b=" << &b << "\n&c=" << &c << "\n";
    std::cout << "bc=" << std::to_string(bc) << ",b=" << std::to_string(*b) << ",c=" << std::to_string(*c) << "\n";


    uint8_t bVal = bc >> 8;
    uint8_t cVal = bc & 0xFF;
    

    OUTPUT:
    &bc=0x50539e
    &b=0x505398
    &c=0x505394
    bc=4660,b=18,c=52
    &bc=0x50539e
    &b=0x505398
    &c=0x505394
    bc=4919,b=19,c=55
    
    */

protected:
    uint8_t mInstructionRegister {};
    uint8_t mInterruptEnable {};

    uint8_t mAccumulator {};
    uint8_t mFlagsRegister {};

    uint16_t mBcRegister {};
    uint16_t mDeRegister {};
    uint16_t mHlRegister {};

    uint16_t mProgramCounter {};
    uint16_t mStackPointer {};
};