#pragma once

#include <cstdint> 

class RegisterFile
{
public:
    RegisterFile();
    ~RegisterFile();

    enum class Register : uint8_t
    {
        register_b = 0b000,

    };

    enum class RegisterPair : uint8_t
    {
        register_bc = 0,
        register_de = 1,
        register_hl = 2
    };
    
protected:
    uint8_t* indexToRegister(uint8_t indexCode) const;

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