#include "CpuCore.h"

#include "OpcodeCycleMap.h"

CpuCore::CpuCore()
{}

void CpuCore::handleCurrentInstruction()
{
    const bool getNewInstruction = (mCurrentInstruction.instructionCycles == 0 || mCurrentInstruction.instructionCycles == mCurrentInstruction.currentCycle);

    if (!getNewInstruction) executeInstruction();

    if ((++mCurrentInstruction.currentCycle == mCurrentInstruction.instructionCycles) || getNewInstruction)
    {
        loadNewInstruction();
    }
}

void CpuCore::loadNewInstruction()
{
    mAddressBus = mRegisters.programCounter();
    const uint8_t instructionCode = mMemoryManager.getMemoryAtAddress(mAddressBus);
    mDataBus = instructionCode;
    
    mCurrentInstruction.instructionCode = instructionCode;
    mCurrentInstruction.currentCycle = 0;
    mCurrentInstruction.instructionCycles = cyclesPerOpcode.at(instructionCode);

    increaseAndStoreProgramCounter();
}

void CpuCore::executeInstruction()
{
    const uint8_t operationType = mCurrentInstruction.instructionCode >> 6;
    
    if (operationType == 0b00)
    {
        handleZeroZeroInstructionBlock();
        return;
    }

    if (operationType == 0b01)
    {
        handleZeroOneInstructionBlock();
        return;
    }

    if (operationType == 0b10)
    {
        handleOneZeroInstructionBlock();
        return;
    }

    if (operationType == 0b11)
    {
        handleOneOneInstructionBlock();
        return;
    }
}


void CpuCore::handleZeroZeroInstructionBlock()
{
    const uint8_t instructionCode = mCurrentInstruction.instructionCode;

    const uint8_t registerId = (instructionCode >> 3) & 0b111;
    const uint8_t mainOperand = instructionCode & 0b111;

    switch (mainOperand)
    {
        case 0b000:
        {
            switch (registerId)
            {
                case 0b000:
                {
                    // No operation
                    return;
                }
                case 0b001:
                {
                    // No operation
                    return;
                }
                default: break;
            }
        }
        case 0b001:
        {
            // TODO
            break;
        }
        case 0b010:
        {
            if (registerId == 0b000)
            {
                if (mCurrentInstruction.currentCycle == 0)
                {
                    mAddressBus = mMemoryManager.getMemoryAtAddress(mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_bc));
                    mDataBus = mRegisters.accumulator();

                    mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);
                }
                else if (mCurrentInstruction.currentCycle == 1)
                {
                    // nothing to do
                }
            }
            else if (registerId == 0b010)
            {
                if (mCurrentInstruction.currentCycle == 0)
                {
                    mAddressBus = mMemoryManager.getMemoryAtAddress(mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_de));
                    mDataBus = mRegisters.accumulator();

                    mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);
                }
                else if (mCurrentInstruction.currentCycle == 1)
                {
                    // nothing to do
                }
            }
            else if (registerId == 0b100)
            {
                if (mCurrentInstruction.currentCycle == 0)
                {
                    mAddressBus = mMemoryManager.getMemoryAtAddress(mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl));
                    mDataBus = mRegisters.accumulator();

                    mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);

                    const uint16_t newRegisterValue = mAlu.incrementRegister(mAddressBus);
                    mRegisters.setBigRegister(Registers::BigRegisterIdentifier::register_hl, newRegisterValue);
                }
                else if (mCurrentInstruction.currentCycle == 1)
                {
                    // nothing to do
                }
            }
            else if (registerId == 0b110)
            {
                if (mCurrentInstruction.currentCycle == 0)
                {
                    mAddressBus = mMemoryManager.getMemoryAtAddress(mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl));
                    mDataBus = mRegisters.accumulator();

                    mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);

                    const uint16_t newRegisterValue = mAlu.decrementRegister(mAddressBus);
                    mRegisters.setBigRegister(Registers::BigRegisterIdentifier::register_hl, newRegisterValue);
                }
                else if (mCurrentInstruction.currentCycle == 1)
                {
                    // nothing to do
                }
            }
            // TODO
            break;
        }
        case 0b011:
        {
            // TODO
            break;
        }
        case 0b100: // increment register
        {
            if (registerId == 0b110) // special case: HL register
            {
                if (mCurrentInstruction.currentCycle == 0)
                {
                    mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
                    mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                }
                else if (mCurrentInstruction.currentCycle == 1)
                {
                    const uint8_t newValue = mAlu.incrementRegister(mDataBus);
                    mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);

                    mRegisters.setFlagValue(Registers::FlagsPosition::zero_flag, newValue == 0);
                    mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
                    mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (newValue >> 3) & 0b1);
                }
                else if (mCurrentInstruction.currentCycle == 2)
                {
                    // do nothing; new instruction will be loaded
                }

                return;
            }
            
            const uint8_t newRegisterValue = mAlu.incrementRegister(mRegisters.smallRegisterValue(registerId));
            mRegisters.setSmallRegister(registerId, newRegisterValue);

            mRegisters.setFlagValue(Registers::FlagsPosition::zero_flag, newRegisterValue == 0);
            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (newRegisterValue >> 3) & 0b1);
            break;
        }
        case 0b101: // decrement register
        {
            if (registerId == 0b110) // special case: HL register
            {
                if (mCurrentInstruction.currentCycle == 0)
                {
                    mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
                    mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                }
                else if (mCurrentInstruction.currentCycle == 1)
                {
                    const uint8_t newValue = mAlu.decrementRegister(mDataBus);
                    mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);

                    mRegisters.setFlagValue(Registers::FlagsPosition::zero_flag, newValue == 0);
                    mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, true);
                    mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (newValue >> 3) & 0b1);
                }
                else if (mCurrentInstruction.currentCycle == 2)
                {
                    // do nothing; new instruction will be loaded
                }

                return;
            }
            
            const uint8_t newRegisterValue = mAlu.decrementRegister(mRegisters.smallRegisterValue(registerId));
            mRegisters.setSmallRegister(registerId, newRegisterValue);

            mRegisters.setFlagValue(Registers::FlagsPosition::zero_flag, newRegisterValue == 0);
            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, true);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (newRegisterValue >> 3) & 0b1);
            break;
        }
        case 0b110: // load incoming data to register
        {
            if (registerId == 0b110) // special case: HL register
            {
                if (mCurrentInstruction.currentCycle == 0)
                {
                    mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                    increaseAndStoreProgramCounter();
                }
                else if (mCurrentInstruction.currentCycle == 1)
                {
                    mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
                    mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);
                }
                else if (mCurrentInstruction.currentCycle == 2)
                {
                    // do nothing; we're done
                }
                break;
            }

            if (mCurrentInstruction.currentCycle == 0)
            {
                mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                increaseAndStoreProgramCounter();
            }
            else if (mCurrentInstruction.currentCycle == 1)
            {
                uint8_t registerValue = mRegisters.smallRegisterValue(registerId);
                mAlu.assignRegisterValue(mDataBus, registerValue);

                mRegisters.setSmallRegister(registerId, registerValue);
            }

            break;
        }
        case 0b111: // some TODOs
        {
            switch (registerId)
            {
                case 0b000: // RLCA TODO
                {
                    // TODO
                    return;
                }
                case 0b001: // RRCA TODO
                {
                    // TODO
                    return;
                }
                case 0b010: // RLA TODO
                {
                    // TODO
                    return;
                }
                case 0b011: // RRA TODO
                {
                    // TODO
                    return;
                }
                case 0b100: // DAA TODO
                {
                    // TODO
                    return;
                }
                case 0b101: // complement accumulator
                {
                    uint8_t accumulator = mRegisters.accumulator();
                    mAlu.flipValue(accumulator);
                    mRegisters.setAccumulator(accumulator);

                    mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, true);
                    mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, true);
                    return;
                }
                case 0b110: // set carry flag
                {
                    mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
                    mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, false);
                    mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, true);
                    return;
                }
                case 0b111: // complement carry flag
                {
                    mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
                    mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, false);

                    const bool carryFlag = mRegisters.flagValue(Registers::FlagsPosition::carry_flag);
                    mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, (carryFlag == false));
                    return;
                }
            }
            // TODO; various instructions
            break;
        }
        default:
        {
            // nothing happens
            break;
        }
    }

/*
    // TODO merge with mainOperand switch statement
    switch (instructionCode)
    {
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
                mDataBus = mRegisters.accumulator();
                mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);
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
        case 0x2F: // complement accumulator
        {
            // instruction takes only one cycle; no need to check
            uint8_t accumulator = mRegisters.accumulator();
            mAlu.flipValue(accumulator);
            mRegisters.setAccumulator(accumulator);

            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, true);
            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, true);
            break;
        }
    }

    */
}

void CpuCore::handleZeroOneInstructionBlock()
{
    const uint8_t instructionCode = mCurrentInstruction.instructionCode;

    const uint8_t firstOperand = (instructionCode >> 3) & 0b111;
    const uint8_t secondOperand = instructionCode & 0b111;

    if (firstOperand == 0b110 && secondOperand == 0b110) // special case: HALT
    {
        mRegisters.setInterruptEnable(0);
        return;
    }

    if (firstOperand == 0b110) // special case: load into memory data at HL address
    {
        if (mCurrentInstruction.currentCycle == 0)
        {
            mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
            mDataBus = mRegisters.smallRegisterValue(secondOperand);

            mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);
        }
        else if (mCurrentInstruction.currentCycle == 1)
        {
            // nothing left to do; let the cycle play out
        }

        return;
    }

    if (secondOperand == 0b110) // sepcial case: get memory data at HL adress
    {
        if (mCurrentInstruction.currentCycle == 0)
        {
            mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
            mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
        }
        else if (mCurrentInstruction.currentCycle == 1)
        {
            uint8_t registerValue = mRegisters.smallRegisterValue(firstOperand);
            mAlu.assignRegisterValue(mDataBus, registerValue);

            mRegisters.setSmallRegister(secondOperand, registerValue);
        }
            
        return;
    }

    // if neither of the operands is equal to 0b110, other operations are simple one-cycle 'load A into B' instructions

    uint8_t registerValue = mRegisters.smallRegisterValue(firstOperand);
    const uint8_t srcRegister = mRegisters.smallRegisterValue(secondOperand);

    mAlu.assignRegisterValue(srcRegister, registerValue);
    mRegisters.setSmallRegister(firstOperand, registerValue);

    return;
}

void CpuCore::handleOneZeroInstructionBlock()
{
    const uint8_t instructionCode = mCurrentInstruction.instructionCode;

    // Arithmetic operations with the accumulator register
    const uint8_t accumulatorValue = mRegisters.accumulator();
    const uint8_t arithmeticOperation = (instructionCode >> 3) & 0b111;
    const uint8_t registerOperand = instructionCode & 0b111;

    if (registerOperand == 0b110) // HL register instructions
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

            setFlagsAfterArithmeticOperation(arithmeticOperation, result);
            return;
        }
    }

    const uint8_t secondRegister = mRegisters.smallRegisterValue(registerOperand);

    const uint8_t result = mAlu.arithmeticOperation(accumulatorValue, secondRegister, arithmeticOperation);
    mRegisters.setAccumulator(result);

    setFlagsAfterArithmeticOperation(arithmeticOperation, result);
    return;
}

void CpuCore::handleOneOneInstructionBlock()
{
    // TODO

    const uint8_t instructionCode = mCurrentInstruction.instructionCode;
    if (instructionCode == 0x37) // set carry flag
    {
        mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, true);
        mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, false);
        mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);

        increaseAndStoreProgramCounter();
        return;
    }

    if (instructionCode == 0x3F) // complement carry flag
    {
        const bool carryFlag = mAlu.flipValue(mRegisters.flagValue(Registers::FlagsPosition::carry_flag));

        mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, carryFlag);
        mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, false);
        mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);

        increaseAndStoreProgramCounter();
        return;
    }


    if (instructionCode == 0xE9) // jump to HL
    {
        mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
        mDataBus = mCurrentInstruction.instructionCode;
        
        increaseAndStoreProgramCounter();
        return;
    }
}

void CpuCore::increaseAndStoreProgramCounter()
{
    mIdu.increaseValue(mAddressBus);
    mRegisters.setProgramCounter(mAddressBus);
}

void CpuCore::setFlagsAfterArithmeticOperation(uint8_t operationType, uint8_t result)
{
    switch (static_cast<AluOperationType>(operationType))
    {
        case AluOperationType::add:
        case AluOperationType::add_plus_carry:
        {
            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (((result >> 3) & 0b1) == 0b1));
            mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, (((result >> 7) & 0b1) == 0b1));
            break;
        }
        case AluOperationType::subtract:
        case AluOperationType::subtract_plus_carry:
        case AluOperationType::compare:
        {
            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, true);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (((result >> 3) & 0b1) == 0b1));
            mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, (((result >> 7) & 0b1) == 0b1));
            break;
        }
        case AluOperationType::logical_and:
        {
            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, true);
            mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, false);
            break;
        }
        case AluOperationType::logical_xor:
        case AluOperationType::logical_or:
        {
            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, false);
            break;
        }
    }

    mRegisters.setFlagValue(Registers::FlagsPosition::zero_flag, (result == 0));
}