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

    uint8_t singleRegisterValue(uint8_t identifier) const;
    uint16_t combinedRegisterValue(uint8_t identifier) const;

    uint8_t& singleRegister(uint8_t identifier);

    // setters
    void setSingleRegister(uint8_t identifier, uint8_t value);
    void setCombinedRegister(uint8_t identifier, uint16_t value);

    void setProgramCounter(uint16_t newValue);

protected:
    uint8_t mInstructionRegister {};
    uint8_t mInterruptEnable {};

    uint8_t mAccumulator {};
    uint8_t mFlagsRegister {};

    uint8_t mBRegister {};
    uint8_t mCRegister {};
    uint8_t mDRegister {};
    uint8_t mERegister {};
    uint8_t mHRegister {};
    uint8_t mLRegister {};

    uint16_t mProgramCounter {};
    uint16_t mStackPointer {};
};