#include "CpuCore.h"

#include "OpcodeCycleMap.h"

CpuCore::CpuCore()
{}

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

bool CpuCore::handleCurrentInstruction()
{
    if (mCurrentInstruction.instructionCycles == 0 || mCurrentInstruction.instructionCycles == mCurrentInstruction.currentCycle)
    {
        return true;
    }

    executeInstruction();

    return (++mCurrentInstruction.currentCycle == mCurrentInstruction.instructionCycles);
}

void CpuCore::executeInstruction()
{
    const uint8_t opcode = mCurrentInstruction.instructionCode;

    switch (opcode)
    {
    case 0x00:
    {
        // do nothing
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
            mAddressBus = mRegisters.combinedRegisterValue(0);
            mDataBus = mRegisters.singleRegisterValue(1);

        }
        
    }
    case 0x41:
    {
        // instruction takes only one cycle; no need to check
        mAddressBus = mRegisters.programCounter();
        mIdu.increaseValue(mAddressBus);
        // mAlu.assignRegisterValue(2, 1);
        mRegisters.setProgramCounter(mAddressBus);
        // TODO
        break;
    }
        // TODO
    }
}