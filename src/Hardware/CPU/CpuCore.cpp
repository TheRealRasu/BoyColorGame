#include "CpuCore.h"

#include "OpcodeCycleMap.h"

CpuCore::CpuCore()
{}

bool CpuCore::handleCurrentInstruction()
{
    if (mCurrentInstruction.instructionCycles == 0 || mCurrentInstruction.instructionCycles == mCurrentInstruction.currentCycle)
    {
        return true;
    }

    executeInstruction();

    return (++mCurrentInstruction.currentCycle == mCurrentInstruction.instructionCycles);
}

void CpuCore::decodeNewInstruction(uint8_t newInstruction)
{
    mCurrentInstruction.instructionCode = newInstruction;
    mCurrentInstruction.currentCycle = 0;

    const bool instructionCondition = checkInstructionCondition(newInstruction);
    mCurrentInstruction.instructionCycles = instructionCondition ? cyclesPerOpcode.at(newInstruction).first : cyclesPerOpcode.at(newInstruction).second;
}

bool CpuCore::checkInstructionCondition(uint8_t instruction)
{
    switch (instruction)
    {
        case 0x20: return true; // TODO
        case 0x28: return true; // TODO
        case 0x30: return true; // TODO
        case 0x38: return true; // TODO
        case 0xC0: return true; // TODO
        case 0xC2: return true; // TODO
        case 0xC4: return true; // TODO
        case 0xC8: return true; // TODO
        case 0xCA: return true; // TODO
        case 0xCC: return true; // TODO
        case 0xD0: return true; // TODO
        case 0xD2: return true; // TODO
        case 0xD4: return true; // TODO
        case 0xD8: return true; // TODO
        case 0xDA: return true; // TODO
        case 0xDC: return true; // TODO
        default:
        {
            // do nothing and return true
            break;
        }
    }
    
    return true;
}

void CpuCore::executeInstruction()
{
    const uint8_t instructionCode = mCurrentInstruction.instructionCode;
    
    switch (instructionCode >> 6)
    {
    case 0b00:
    {
        // various instructions
    }
    case 0b01:
    {
        if (instructionCode == 0x76) // special case: HALT
        {
            mAddressBus = mRegisters.programCounter();
            mDataBus = mCurrentInstruction.instructionCode;

            mRegisters.setInterruptEnable(0);

            mIdu.increaseValue(mAddressBus);
            mRegisters.setProgramCounter(mAddressBus);
        }

        // load 8-bit value and store it   
    }
    case 0b10:
    {
        // Arithmetic operations regarding register A
    }
    case 0b11:
    {
        
    }
    default: // we already covered all cases, but the compiler needs to nitpick
    {
        break;
    }
    }

    switch (instructionCode)
    {
    case 0x00:
    {
        mAddressBus = mRegisters.programCounter();
        mDataBus = mCurrentInstruction.instructionCode;

        mIdu.increaseValue(mAddressBus);

        mRegisters.setProgramCounter(mAddressBus);

        break;
    }
    case 0x01:
    {
        // TODO
        break;
    }
    case 0x02:
    {
        if (mCurrentInstruction.currentCycle == 0)
        {
            mAddressBus = mRegisters.combinedRegisterValue(Registers::CombinedRegister::register_bc);
            // mDataBus = mRegisters.singleRegisterValue(1);
        }
        else if (mCurrentInstruction.currentCycle == 1)
        {
            mAddressBus = mRegisters.programCounter();
            mDataBus = mCurrentInstruction.instructionCode;

            mIdu.increaseValue(mAddressBus);
            mRegisters.setInstructionRegister(mDataBus);
            mRegisters.setProgramCounter(mAddressBus);
        }
        
    }
    case 0x41: // load register (register)
    {
        // instruction takes only one cycle; no need to check
        mAddressBus = mRegisters.programCounter();
        mDataBus = mCurrentInstruction.instructionCode;

        mIdu.increaseValue(mAddressBus);
        // mAlu.assignRegisterValue(2, 1);

        mRegisters.setProgramCounter(mAddressBus);
        mRegisters.setInstructionRegister(mDataBus);
        // TODO
        break;
    }
    case 0x2F: // complement accumulator
    {
        // instruction takes only one cycle; no need to check
        mAddressBus = mRegisters.programCounter();
        mDataBus = mCurrentInstruction.instructionCode;

        mIdu.increaseValue(mAddressBus);
        uint8_t accumulator = mRegisters.accumulator();
        mAlu.flipRegister(accumulator);
        mRegisters.setAccumulator(accumulator);
        // TODO set flags
        break;
    }
    case 0xE9: // jump to HL
    {
        mAddressBus = mRegisters.combinedRegisterValue(Registers::CombinedRegister::register_hl);
        mDataBus = mCurrentInstruction.instructionCode;
        
        mIdu.increaseValue(mAddressBus);
        mRegisters.setProgramCounter(mAddressBus);
    }
        // TODO
    }
}

CpuCore::Instruction CpuCore::instruction() const
{
    return mCurrentInstruction;
}

uint16_t CpuCore::programCounter() const
{
    return mRegisters.programCounter();
}