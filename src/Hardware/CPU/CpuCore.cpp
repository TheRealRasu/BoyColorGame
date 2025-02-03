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
    mCurrentInstruction.instructionCode = mMemoryManager.getMemoryAtAddress(mRegisters.programCounter());
    mCurrentInstruction.currentCycle = 0;

    const bool instructionCondition = checkInstructionCondition(newInstruction);
    mCurrentInstruction.instructionCycles = instructionCondition ? cyclesPerOpcode.at(newInstruction).first : cyclesPerOpcode.at(newInstruction).second;

    increaseAndStoreProgramCounter();
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

    const uint8_t operationType = instructionCode >> 6;
    
    switch (operationType)
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

        const uint8_t firstOperand = (instructionCode >> 3) & 0b111;
        const uint8_t secondOperand = instructionCode & 0b111;


        // load 8-bit value and store it   
    }
    case 0b10:
    {
        // Arithmetic operations with the accumulator register
        const uint8_t arithmeticOperation = (instructionCode >> 3) & 0b111;
        const uint8_t registerOperand = instructionCode & 0b111;
        const uint8_t accumulatorValue = mRegisters.accumulator();

        if (registerOperand == 0x06)
        {
            if (mCurrentInstruction.currentCycle == 0)
            {
                mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
                mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                return;
            }
            else if (mCurrentInstruction.currentCycle == 1)
            {
                const uint8_t result = mAlu.arithmeticOperation(accumulatorValue, mDataBus, arithmeticOperation);
                mRegisters.setAccumulator(result);

                // TODO flags

                mAddressBus = mRegisters.programCounter();
                mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                uint16_t programCounter = mRegisters.programCounter();
                mIdu.increaseValue(programCounter);
                mRegisters.setProgramCounter(programCounter);
            }
        }

        const uint8_t secondRegister = mRegisters.smallRegisterValue(registerOperand);
    }
    case 0b11:
    {
        // TODO
        if (instructionCode == 0x3F) // complement carry flag
        {
            mAddressBus = mRegisters.programCounter();
            mDataBus = mCurrentInstruction.instructionCode;

            const bool carryFlag = mAlu.flipValue(mRegisters.flagValue(Registers::FlagsPosition::carry_flag));

            mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, carryFlag);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);

            increaseAndStoreProgramCounter();
        }
        if (instructionCode == 0x3F) // set carry flag
        {
            mAddressBus = mRegisters.programCounter();
            mDataBus = mCurrentInstruction.instructionCode;

            mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, true);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);

            increaseAndStoreProgramCounter();
        }
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

        increaseAndStoreProgramCounter();

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
            mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_bc);
            mDataBus = mRegisters.smallRegisterValue(1);
        }
        else if (mCurrentInstruction.currentCycle == 1)
        {
            mAddressBus = mRegisters.programCounter();
            mDataBus = mCurrentInstruction.instructionCode;

            increaseAndStoreProgramCounter();

            mRegisters.setInstructionRegister(mDataBus);
        }
        // TODO?
        break;
    }
    case 0x41: // load register (register)
    {
        // instruction takes only one cycle; no need to check
        mAddressBus = mRegisters.programCounter();
        mDataBus = mCurrentInstruction.instructionCode;

        // mAlu.assignRegisterValue(2, 1);

        increaseAndStoreProgramCounter();

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
        mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
        mDataBus = mCurrentInstruction.instructionCode;
        
        increaseAndStoreProgramCounter();
    }
        // TODO
    }
}

void CpuCore::increaseAndStoreProgramCounter()
{
    mIdu.increaseValue(mAddressBus);
    mRegisters.setProgramCounter(mAddressBus);
}

void CpuCore::registerAddition()
{
    const uint8_t instructionCode = mCurrentInstruction.instructionCode;

    const uint8_t arithmeticOperation = (instructionCode >> 3) & 0b111;
    const uint8_t registerOperand = instructionCode & 0b111;
    const uint8_t accumulatorValue = mRegisters.accumulator();

    const uint8_t result = mAlu.arithmeticOperation(accumulatorValue, mDataBus, arithmeticOperation);
    mRegisters.setAccumulator(result);

    mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, (result >> 7) & 0b1);
    mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (result >> 3) & 0b1);
    mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
    mRegisters.setFlagValue(Registers::FlagsPosition::zero_flag, result == 0);
}