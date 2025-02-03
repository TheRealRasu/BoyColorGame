#pragma once

#include <cstdint> 

class Registers
{
public:
    Registers();
    ~Registers();

    enum class BigRegisterIdentifier : uint8_t
    {
        register_bc = 0,
        register_de = 1,
        register_hl = 2,
        stack_pointer = 3,
        program_counter = 4
    };
    
    enum class FlagsPosition : uint8_t
    {
        carry_flag = 4,
        half_carry_flag = 5,
        subtraction_flag = 6,
        zero_flag = 7
    };

    // getters
    uint16_t programCounter() const;
    uint8_t accumulator() const;

    bool flagValue(FlagsPosition pos) const;

    uint8_t smallRegisterValue(const uint8_t identifier) const;
    uint16_t bigRegisterValue(const BigRegisterIdentifier identifier) const;

    // setters
    void setProgramCounter(const uint16_t newValue);
    void setAccumulator(const uint8_t newValue);
    void setInstructionRegister(const uint8_t instruction);
    void setInterruptEnable(const uint8_t newValue);

    void setFlagValue(FlagsPosition pos, bool value);

    void setSmallRegister(const uint8_t identifier, const uint8_t value);
    void setBigRegister(const BigRegisterIdentifier identifier, const uint16_t value);

protected:
    uint8_t mInstructionRegister {};
    uint8_t mInterruptEnable {};

    uint8_t mAccumulator {};
    uint8_t mFlagsRegister {};

    uint16_t mBRegister {};
    uint16_t mCRegister {};
    uint16_t mDRegister {};
    uint16_t mERegister {};
    uint16_t mHRegister {};
    uint16_t mLRegister {};

    uint16_t mProgramCounter {};
    uint16_t mStackPointer {};
};