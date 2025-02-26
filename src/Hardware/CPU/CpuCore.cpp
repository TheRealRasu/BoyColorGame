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
    mCurrentInstruction.conditionMet = false;
    mCurrentInstruction.temporalData.clear();

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
        case 0b000: // TODO
        {
            switch (registerId)
            {
                case 0b000:
                {
                    // No operation
                    return;
                }
                case 0b001: // load from stack pointer
                {
                    if ((mCurrentInstruction.currentCycle == 0) || (mCurrentInstruction.currentCycle == 1))
                    {
                        mAddressBus = mRegisters.programCounter();
                        mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);

                        mCurrentInstruction.temporalData.push_back(mDataBus);
                    }
                    else if (mCurrentInstruction.currentCycle == 2)
                    {
                        const std::vector<uint8_t>& tempData = mCurrentInstruction.temporalData;

                        mAddressBus = tempData.at(0) + (tempData.at(1) << 8);
                        mDataBus = mRegisters.stackPointer() & 0xFF;

                        mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);
                        mAddressBus++;
                    }
                    else if (mCurrentInstruction.currentCycle == 3)
                    {
                        mDataBus = (mRegisters.stackPointer() >> 8) & 0xFF;
                        mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);
                    }
                    else if (mCurrentInstruction.currentCycle == 4)
                    {
                        // nothing left to do
                    }
                    return;
                }
                case 0b010: // TODO Stop. Consult with CPU documents
                {
                    return;
                }
                case 0b011: // JR e
                {
                    if (mCurrentInstruction.currentCycle == 0)
                    {
                        mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                        mCurrentInstruction.temporalData.push_back(mDataBus);

                        increaseAndStoreProgramCounter();
                    }
                    else if (mCurrentInstruction.currentCycle == 1)
                    {
                        std::vector<uint8_t>& tempDataVec = mCurrentInstruction.temporalData;
                        mAddressBus = mRegisters.programCounter();
                        uint8_t& tempData = tempDataVec.at(0);
                        const bool dataBusSigned = (tempData >> 7) & 0b1;

                        const uint8_t lowByte = mAddressBus & 0xFF;
                        tempData += lowByte;

                        const bool resultSigned = (tempData >> 7) & 0b1;

                        int8_t extraValue = 0; 
                        if (resultSigned && !dataBusSigned)
                        {
                            extraValue = 1;
                        }
                        else if (!resultSigned && dataBusSigned)
                        {
                            extraValue = -1;
                        }

                        const uint8_t nextTempData = ((mAddressBus >> 8) & 0xFF) + extraValue;
                        tempDataVec.push_back(nextTempData);
                    }
                    else if (mCurrentInstruction.currentCycle == 2)
                    {
                        const std::vector<uint8_t>& tempData = mCurrentInstruction.temporalData;
                        const uint16_t newProgramCounter = tempData.at(0) + (tempData.at(1) << 8);

                        mRegisters.setProgramCounter(newProgramCounter);
                    }
                    return;
                }
                case 0b100: // JR NZ, e
                case 0b101: // JR Z, e
                case 0b110: // JR NC, e
                case 0b111: // JR C, e
                {
                    if (mCurrentInstruction.currentCycle == 0)
                    {
                        mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                        mCurrentInstruction.temporalData.push_back(mDataBus);

                        const Registers::FlagCondition operandCode = static_cast<Registers::FlagCondition>(registerId & 0b11);
                        mCurrentInstruction.conditionMet = mRegisters.checkFlagCondition(operandCode);

                        if (mCurrentInstruction.conditionMet)
                        {
                            mCurrentInstruction.instructionCycles = 3;
                        }

                        increaseAndStoreProgramCounter();
                    }
                    else if (mCurrentInstruction.currentCycle == 1)
                    {
                        if (mCurrentInstruction.conditionMet == false) return;

                        const uint8_t pcLow = mRegisters.programCounter() & 0xFF;
                        const uint8_t firstResult = mCurrentInstruction.temporalData.at(0) + pcLow;
                        mCurrentInstruction.temporalData.at(0) = firstResult;

                        const bool carryBit = (mDataBus >> 7) & 0b1;
                        const bool resultCarry = (firstResult >> 7) & 0b1;
                        
                        uint8_t adj = 0;
                        if (resultCarry && !carryBit)
                        {
                            adj = 1;
                        }
                        else if (!resultCarry && carryBit)
                        {
                            adj = -1;
                        }

                        const uint8_t secondResult = ((mRegisters.programCounter() >> 8) & 0xFF) + adj;
                        mCurrentInstruction.temporalData.push_back(secondResult);
                    }
                    else if (mCurrentInstruction.currentCycle == 2)
                    {
                        const std::vector<uint8_t>& tempData = mCurrentInstruction.temporalData;
                        const uint16_t newProgramCounter = tempData.at(0) + (tempData.at(1) << 8);

                        mRegisters.setProgramCounter(newProgramCounter);
                    }
                }
                default: break;
            }
        }
        case 0b001: // TODO
        {
            if (registerId & 0b1) // TODO: ADD HL, XX
            {
                if (mCurrentInstruction.currentCycle == 0)
                {
                    mAddressBus = 0x0000;

                    // addition of lsb's
                    const uint16_t hlValue = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);

                if (uint8_t identifier = (registerId >> 1) & 0b11; identifier == 0b00)
                {
                    mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_bc);
                }
                else if (identifier == 0b01)
                {
                    mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_de);
                }
                else if (identifier == 0b01)
                {
                    mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
                }
                else if (identifier == 0b01)
                {
                    mAddressBus = mRegisters.stackPointer();
                }

                    // TODO: first addition
                    mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
                }
                else if (mCurrentInstruction.currentCycle == 1)
                {
                    uint8_t hValue = mRegisters.smallRegisterValue(0b100);

                    // TODO: second addition
                    mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
                }
            }
            else // LD rr, nn
            {
                if (mCurrentInstruction.currentCycle == 0 || mCurrentInstruction.currentCycle == 1)
                {
                    mAddressBus = mRegisters.programCounter();
                    mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);

                    mCurrentInstruction.temporalData.push_back(mDataBus);
                    increaseAndStoreProgramCounter();
                    break;
                }
                else if (mCurrentInstruction.currentCycle == 2)
                {
                    const std::vector<uint8_t>& tempData = mCurrentInstruction.temporalData;
                    const uint16_t newValue = tempData.at(0) + (tempData.at(1) << 8);

                    if (registerId == 0b000)
                    {
                        mRegisters.setBigRegister(Registers::BigRegisterIdentifier::register_bc, newValue);
                    }
                    else if (registerId == 0b010)
                    {
                        mRegisters.setBigRegister(Registers::BigRegisterIdentifier::register_de, newValue);
                    }
                    else if (registerId == 0b100)
                    {
                        mRegisters.setBigRegister(Registers::BigRegisterIdentifier::register_hl, newValue);
                    }
                    else if (registerId == 0b110)
                    {
                        mRegisters.setStackPointer(newValue);
                    }
                }
            }
            break;
        }
        case 0b010: // Load data
        {
            switch (registerId)
            {
                case 0b000:
                case 0b010:
                {
                    mAddressBus = mMemoryManager.getMemoryAtAddress(mRegisters.bigRegisterValue(operandToBigRegister(registerId)));
                    mDataBus = mRegisters.accumulator();

                    mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);
                    return;
                }
                case 0b100:
                {
                    mAddressBus = mMemoryManager.getMemoryAtAddress(mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl));
                    mDataBus = mRegisters.accumulator();

                    mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);

                    uint16_t newRegisterValue = mAddressBus;
                    mAlu.incrementRegister(newRegisterValue);
                    mRegisters.setBigRegister(Registers::BigRegisterIdentifier::register_hl, newRegisterValue);
                    return;
                }
                case 0b110:
                {
                    mAddressBus = mMemoryManager.getMemoryAtAddress(mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl));
                    mDataBus = mRegisters.accumulator();

                    mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);

                    uint16_t newRegisterValue = mAddressBus;
                    mAlu.decrementRegister(newRegisterValue);
                    mRegisters.setBigRegister(Registers::BigRegisterIdentifier::register_hl, newRegisterValue);
                    return;
                }
                case 0b001:
                {
                    if (mCurrentInstruction.currentCycle == 0)
                    {
                        mAddressBus = mMemoryManager.getMemoryAtAddress(mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_bc));
                        mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                    }
                    else
                    {
                        mRegisters.setAccumulator(mDataBus);
                    }
                }
                case 0b011:
                {
                    if (mCurrentInstruction.currentCycle == 0)
                    {
                        mAddressBus = mMemoryManager.getMemoryAtAddress(mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_de));
                        mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                    }
                    else
                    {
                        mRegisters.setAccumulator(mDataBus);
                    }
                }
                case 0b101:
                {
                    if (mCurrentInstruction.currentCycle == 0)
                    {
                        uint16_t hlValue = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
                        mAddressBus = mMemoryManager.getMemoryAtAddress(hlValue);
                        mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);

                        mIdu.increaseValue(hlValue);
                        mRegisters.setBigRegister(Registers::BigRegisterIdentifier::register_hl, hlValue);
                    }
                    else
                    {
                        mRegisters.setAccumulator(mDataBus);
                    }
                }
                case 0b111:
                {
                    if (mCurrentInstruction.currentCycle == 0)
                    {
                        uint16_t hlValue = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
                        mAddressBus = mMemoryManager.getMemoryAtAddress(hlValue);
                        mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);

                        mIdu.decreaseValue(hlValue);
                        mRegisters.setBigRegister(Registers::BigRegisterIdentifier::register_hl, hlValue);
                    }
                    else
                    {
                        mRegisters.setAccumulator(mDataBus);
                    }
                }
            }

            return;
        }
        case 0b011: // INC/DEC rr
        {
            if (mCurrentInstruction.currentCycle == 0)
            {
                if (uint8_t identifier = (registerId >> 1) & 0b11; identifier == 0b00)
                {
                    mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_bc);
                }
                else if (identifier == 0b01)
                {
                    mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_de);
                }
                else if (identifier == 0b01)
                {
                    mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
                }
                else if (identifier == 0b01)
                {
                    mAddressBus = mRegisters.stackPointer();
                }

                if (registerId & 0b1)
                {
                    mIdu.increaseValue(mAddressBus);
                }
                else
                {
                    mIdu.decreaseValue(mAddressBus);
                }

                if (uint8_t identifier = (registerId >> 1) & 0b11; identifier == 0b00)
                {
                    mRegisters.setBigRegister(Registers::BigRegisterIdentifier::register_bc, mAddressBus);
                }
                else if (identifier == 0b01)
                {
                    mRegisters.setBigRegister(Registers::BigRegisterIdentifier::register_de, mAddressBus);
                }
                else if (identifier == 0b01)
                {
                    mRegisters.setBigRegister(Registers::BigRegisterIdentifier::register_hl, mAddressBus);
                }
                else if (identifier == 0b01)
                {
                    mRegisters.setStackPointer(mAddressBus);
                }
            }

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
                    uint8_t newValue = mDataBus;
                    mAlu.incrementRegister(newValue);
                    mMemoryManager.writeToMemoryAddress(mAddressBus, newValue);

                    mRegisters.setFlagValue(Registers::FlagsPosition::zero_flag, newValue == 0);
                    mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
                    mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (newValue >> 3) & 0b1);
                }

                return;
            }
            
            uint8_t newRegisterValue = mRegisters.smallRegisterValue(registerId);
            mAlu.incrementRegister(newRegisterValue);
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
                    uint8_t newValue = mDataBus;
                    mAlu.decrementRegister(newValue);
                    mMemoryManager.writeToMemoryAddress(mAddressBus, newValue);

                    mRegisters.setFlagValue(Registers::FlagsPosition::zero_flag, newValue == 0);
                    mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, true);
                    mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (newValue >> 3) & 0b1);
                }

                return;
            }
            
            uint8_t newRegisterValue = mRegisters.smallRegisterValue(registerId);
            mAlu.decrementRegister(newRegisterValue);
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
        case 0b111: // one TODO
        {
            switch (registerId)
            {
                case 0b000: // RLCA
                case 0b001: // RRCA
                case 0b010: // RLA
                case 0b011: // RRA
                {
                    uint8_t accumulatorValue = mRegisters.accumulator();
                    bool carryFlag = mRegisters.flagValue(Registers::FlagsPosition::carry_flag);
                    
                    const bool rotateRight = registerId & 0b1;
                    const bool throughCarry = (registerId >> 1) & 0b1;

                    mAlu.rotateValue(accumulatorValue, carryFlag, rotateRight, throughCarry);

                    mRegisters.setAccumulator(accumulatorValue);
                    mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, carryFlag);

                    // TODO check if these flag assignments are accurate
                    mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, false);
                    mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
                    return;
                }
                case 0b100: // DAA TODO. Consult with CPU documents
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
}

void CpuCore::handleZeroOneInstructionBlock() // DONE
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

    if (secondOperand == 0b110) // special case: get memory data at HL adress
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

    // if neither of the operands is equal to 0b110, other operations are simple one-cycle 'load X into Y' instructions

    uint8_t registerValue = mRegisters.smallRegisterValue(firstOperand);
    const uint8_t srcRegister = mRegisters.smallRegisterValue(secondOperand);

    mAlu.assignRegisterValue(srcRegister, registerValue);
    mRegisters.setSmallRegister(firstOperand, registerValue);

    return;
}

void CpuCore::handleOneZeroInstructionBlock() // DONE
{
    const uint8_t instructionCode = mCurrentInstruction.instructionCode;

    // Arithmetic operations with the accumulator register
    const uint8_t accumulatorValue = mRegisters.accumulator();
    const Alu::AluOperationType operation = static_cast<Alu::AluOperationType>((instructionCode >> 3) & 0b111);
    const uint8_t registerOperand = instructionCode & 0b111;

    if (registerOperand == 0b110) // special case: HL register instructions
    {
        if (mCurrentInstruction.currentCycle == 0)
        {
            mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
            mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
            return;
        }
        else if (mCurrentInstruction.currentCycle == 1)
        {
            const uint8_t result = mAlu.arithmeticOperation(accumulatorValue, mDataBus, operation);
            
            if (operation != Alu::AluOperationType::compare)
            {
                mRegisters.setAccumulator(result);
            }

            setFlagsAfterArithmeticOperation(operation, result);
            return;
        }
    }

    const uint8_t secondRegister = mRegisters.smallRegisterValue(registerOperand);

    bool additionalFlag {};
    if ((operation == Alu::AluOperationType::add_plus_carry) || (operation == Alu::AluOperationType::subtract_plus_carry))
    {
        additionalFlag = mRegisters.flagValue(Registers::FlagsPosition::carry_flag);
    }

    const uint8_t result = mAlu.arithmeticOperation(accumulatorValue, secondRegister, operation, additionalFlag);
    
    if (operation != Alu::AluOperationType::compare)
    {
        mRegisters.setAccumulator(result);
    }

    setFlagsAfterArithmeticOperation(operation, result);
    return;
}

void CpuCore::handleOneOneInstructionBlock()
{
    const uint8_t instructionCode = mCurrentInstruction.instructionCode;

    const uint8_t firstOperand = (instructionCode >> 3) & 0b111;
    const uint8_t mainOperand = instructionCode & 0b111;

    switch (mainOperand)
    {
        case 0b000: // various; TODO
        {
            if (firstOperand == 0b100) // LDH (n)
            {
                switch (mCurrentInstruction.currentCycle)
                {
                    case 0:
                    {
                        const uint8_t lowByte = mMemoryManager.getMemoryAtAddress(mAddressBus);
                        mCurrentInstruction.temporalData.push_back(lowByte);

                        increaseAndStoreProgramCounter();
                        break;
                    }
                    case 1:
                    {
                        mAddressBus = (0xFF << 8) + mCurrentInstruction.temporalData.at(0);
                        mDataBus = mRegisters.accumulator();
                        mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);
                        break;
                    }
                    default:
                    {
                        // nothing left to do
                        break;
                    }
                }
            }
            else if (firstOperand == 0b101) // TODO ADD SP, e
            {
                if (const uint8_t curr = mCurrentInstruction.currentCycle; curr == 0)
                {
                    mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                    increaseAndStoreProgramCounter();
                }
                else if (curr == 1)
                {
                    mAddressBus = 0x0000;

                    mDataBus += (mRegisters.stackPointer() & 0xFF);

                    mRegisters.setFlagValue(Registers::FlagsPosition::zero_flag, false);
                    mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
                    mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (mDataBus >> 3 & 0b1));
                    mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, (mDataBus >> 7 & 0b1));

                    mCurrentInstruction.temporalData.push_back(mDataBus);
                }
                else if (curr == 2)
                {

                }
                // TODO ADD SP, e
            }
            else if (firstOperand == 0b110) // LDH A, (n)
            {
                switch (mCurrentInstruction.currentCycle)
                {
                    case 0:
                    {
                        mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);

                        increaseAndStoreProgramCounter();
                        break;
                    }
                    case 1:
                    {
                        mAddressBus = (0xFF << 8) + mDataBus;
                        mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                        break;
                    }
                    case 2:
                    {
                        mRegisters.setAccumulator(mDataBus);
                        break;
                    }
                    default:
                    {
                        // nothing left to do
                        break;
                    }
                }
            }
            else if (firstOperand == 0b111) // LD HL, SP+e
            {
                // TODO LD HL, SP+e
            }
            else // RET condition
            {
                conditionalReturnFromFunction();
            }
            break;
        }
        case 0b001: // pop from stack + 4 special cases, TODO
        {
            if (firstOperand == 0b001) // RET - TODO
            {
                return;
            }
            else if (firstOperand == 0b011) // RETI - TODO
            {
                return;
            }
            else if (firstOperand == 0b101) // jump to HL
            {
                mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
                mRegisters.setProgramCounter(mAddressBus);
                return;
            }
            else if (firstOperand == 0b111) // load stack pointer from HL
            {
                mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
                mRegisters.setStackPointer(mAddressBus);
                return;
            }

            // pop from stack
            switch (mCurrentInstruction.currentCycle)
            {
                case 0:
                case 1:
                {
                    mAddressBus = mRegisters.stackPointer();
                    mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);

                    mCurrentInstruction.temporalData.push_back(mDataBus);

                    increaseAndStoreStackPointer();
                    break;
                }
                case 2:
                {
                    const std::vector<uint8_t>& temporalData = mCurrentInstruction.temporalData;
                    const uint16_t newValue = temporalData.at(0) + (temporalData.at(1) << 8);
                    
                    mRegisters.setBigRegister(operandToBigRegister(firstOperand), newValue);
                    break;
                }
                default:
                {
                    // do nothing; we're done here
                    break;
                }
            }

            break;
        }
        case 0b010: // TODO
        {
            return;
        }
        case 0b011: // TODO
        {
            if (firstOperand == 0b000) // JP nn
            {
                switch (mCurrentInstruction.currentCycle)
                {
                    case 0:
                    case 1:
                    {
                        mAddressBus = mRegisters.programCounter();
                        mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);

                        mCurrentInstruction.temporalData.push_back(mDataBus);

                        increaseAndStoreProgramCounter();
                        break;
                    }
                    case 2:
                    {
                        mAddressBus = 0x0000;

                        const std::vector<uint8_t>& tempData = mCurrentInstruction.temporalData;
                        const uint16_t newProgramCounter = tempData.at(0) + (tempData.at(1) << 8);

                        mRegisters.setProgramCounter(newProgramCounter);
                        break;
                    }
                    default:
                    {
                        // nothing left to do
                    }
                }
            }
            else if (firstOperand == 0b001)
            {
                // 0xCB TODO
            }
            else if (firstOperand == 0b110) // DI
            {
                mRegisters.setInterruptEnable(0u);
            }
            else if (firstOperand == 0b111) // EI
            {
                mRegisters.setInterruptEnable(1u);
            }
            return;
        }
        case 0b100: // Call condition
        {
            if ((firstOperand >> 2) & 0b1) return; // undefined actions

            conditionalFunctionCall();
            break;
        }
        case 0b101: // PUSH + one special case
        {
            switch (firstOperand)
            {
                case 0b011:
                case 0b101:
                case 0b111:
                {
                    // no operation
                    return;
                }
                case 0b001: // CALL nn
                {
                    callAddress();
                    break;
                }
                case 0b000:
                case 0b010:
                case 0b100:
                case 0b110:
                {
                    // Push to Stack
                    switch (mCurrentInstruction.currentCycle)
                    {
                        case 0:
                        {
                            mAddressBus = mRegisters.stackPointer();
                            decreaseAndStoreStackPointer();
                            return;
                        }
                        case 1:
                        {
                            mAddressBus = mRegisters.stackPointer();
                            mDataBus = mRegisters.bigRegisterValue(operandToBigRegister(firstOperand)) >> 8;

                            mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);
                            decreaseAndStoreStackPointer();
                            return;
                        }
                        case 2:
                        {
                            mAddressBus = mRegisters.stackPointer();
                            mDataBus = mRegisters.bigRegisterValue(operandToBigRegister(firstOperand)) & 0xFF;

                            mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);
                            return;
                        }
                        default:
                        {
                            // done here
                            return;
                        }
                    }
                    break;
                }
            }
        }
        case 0b110: // immediate data operation
        {
            if (mCurrentInstruction.currentCycle == 0)
            {
                mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                increaseAndStoreProgramCounter();
            }
            else if (mCurrentInstruction.currentCycle == 1)
            {
                const uint8_t accumulatorValue = mRegisters.accumulator();
                const Alu::AluOperationType operation = static_cast<Alu::AluOperationType>(firstOperand);
                const uint8_t result = mAlu.arithmeticOperation(accumulatorValue, mDataBus, operation);
                
                if (operation != Alu::AluOperationType::compare)
                {
                    mRegisters.setAccumulator(result);
                }

                setFlagsAfterArithmeticOperation(operation, result);
                return;
            }
            break;
        }
        case 0b111: // unconditional function call
        {
            unconditionalFunctionCall();
            break;
        }
    }
}

Registers::BigRegisterIdentifier CpuCore::operandToBigRegister(const uint8_t operand) const
{
    switch (operand)
    {
        case 0b000: return Registers::BigRegisterIdentifier::register_bc;
        case 0b010: return Registers::BigRegisterIdentifier::register_de;
        case 0b100: return Registers::BigRegisterIdentifier::register_hl;
        case 0b110: return Registers::BigRegisterIdentifier::register_af;
        default: return Registers::BigRegisterIdentifier::register_bc;
    }
}

void CpuCore::increaseAndStoreProgramCounter()
{
    mIdu.increaseValue(mAddressBus);
    mRegisters.setProgramCounter(mAddressBus);
}

void CpuCore::decreaseAndStoreProgramCounter()
{
    mIdu.decreaseValue(mAddressBus);
    mRegisters.setProgramCounter(mAddressBus);
}

void CpuCore::increaseAndStoreStackPointer()
{
    mIdu.increaseValue(mAddressBus);
    mRegisters.setStackPointer(mAddressBus);
}

void CpuCore::decreaseAndStoreStackPointer()
{
    mIdu.decreaseValue(mAddressBus);
    mRegisters.setStackPointer(mAddressBus);
}

void CpuCore::setFlagsAfterArithmeticOperation(Alu::AluOperationType operation, uint8_t result)
{
    switch (operation)
    {
        case Alu::AluOperationType::add:
        case Alu::AluOperationType::add_plus_carry:
        {
            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (((result >> 3) & 0b1) == 0b1));
            mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, (((result >> 7) & 0b1) == 0b1));
            break;
        }
        case Alu::AluOperationType::subtract:
        case Alu::AluOperationType::subtract_plus_carry:
        case Alu::AluOperationType::compare:
        {
            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, true);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (((result >> 3) & 0b1) == 0b1));
            mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, (((result >> 7) & 0b1) == 0b1));
            break;
        }
        case Alu::AluOperationType::logical_and:
        {
            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, true);
            mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, false);
            break;
        }
        case Alu::AluOperationType::logical_xor:
        case Alu::AluOperationType::logical_or:
        {
            mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, false);
            mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, false);
            break;
        }
    }

    mRegisters.setFlagValue(Registers::FlagsPosition::zero_flag, (result == 0));
}

void CpuCore::unconditionalFunctionCall()
{
    if (mCurrentInstruction.currentCycle == 0)
    {
        mAddressBus = mRegisters.stackPointer();
        decreaseAndStoreStackPointer();
    }
    else if (mCurrentInstruction.currentCycle == 1)
    {
        mAddressBus = mRegisters.stackPointer();

        mDataBus = (mRegisters.programCounter() >> 8); // most significant byte
        mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);

        decreaseAndStoreStackPointer();
    }
    else if (mCurrentInstruction.currentCycle == 2)
    {
        mAddressBus = mRegisters.stackPointer();

        mDataBus = (mRegisters.programCounter() & 0xFF); // least significant byte
        mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);

        const uint16_t newAddress = mCurrentInstruction.instructionCode & 0x38;
        mRegisters.setProgramCounter(newAddress);
    }
    else if (mCurrentInstruction.currentCycle == 3)
    {
        // nothing left to do
    }
}

void CpuCore::conditionalFunctionCall()
{
    switch (mCurrentInstruction.currentCycle)
    {
        case 0:
        {
            mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
            mCurrentInstruction.temporalData.push_back(mDataBus); // low byte
            increaseAndStoreProgramCounter();

            break;
        }
        case 1:
        {
            mAddressBus = mRegisters.programCounter();
            mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);

            mCurrentInstruction.temporalData.push_back(mDataBus); // high byte
            increaseAndStoreProgramCounter();

            const uint8_t operandCode = (mCurrentInstruction.instructionCode >> 3) & 0b111;
            mCurrentInstruction.conditionMet = mRegisters.checkFlagCondition(static_cast<Registers::FlagCondition>(operandCode));

            if (mCurrentInstruction.conditionMet)
            {
                mCurrentInstruction.instructionCycles = 6;
            }

            break;
        }
        case 2:
        {
            if (mCurrentInstruction.conditionMet == false) break; // nothing left to do

            mAddressBus = mRegisters.stackPointer();
            decreaseAndStoreStackPointer();

            break;
        }
        case 3:
        {
            mAddressBus = mRegisters.stackPointer();
            mDataBus = mRegisters.programCounter() >> 8; // most significant PC byte

            mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);

            decreaseAndStoreStackPointer();

            break;
        }
        case 4:
        {
            mAddressBus = mRegisters.stackPointer();
            mDataBus = mRegisters.programCounter() & 0xFF; // least significant PC byte

            mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);

            const std::vector<uint8_t>& tempData = mCurrentInstruction.temporalData;
            const uint16_t newProgramCounter = tempData.at(0) + (tempData.at(1) << 8);

            mRegisters.setProgramCounter(newProgramCounter);
            break;
        }
        default:
        {
            // nothing left to do
            break;
        }
    }
}

void CpuCore::callAddress()
{
    switch (mCurrentInstruction.currentCycle)
    {
        case 0:
        case 1:
        {
            mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
            mCurrentInstruction.temporalData.push_back(mDataBus);
            increaseAndStoreProgramCounter();
            break;
        }
        case 2:
        {
            mAddressBus = mRegisters.stackPointer();
            decreaseAndStoreStackPointer();
            break;
        }
        case 3:
        {
            mAddressBus = mRegisters.stackPointer();
            mDataBus = mRegisters.programCounter() >> 8;

            mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);

            decreaseAndStoreStackPointer();
            break;
        }
        case 4:
        {
            mAddressBus = mRegisters.stackPointer();
            mDataBus = mRegisters.programCounter() & 0xFF;

            mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);

            const std::vector<uint8_t>& temporalData = mCurrentInstruction.temporalData;
            const uint16_t newValue = temporalData.at(0) + (temporalData.at(1) << 8);
            mRegisters.setProgramCounter(newValue);
            break;
        }
        default:
        {
            // done here
            return;
        }
    }
}

void CpuCore::conditionalReturnFromFunction()
{
    switch (mCurrentInstruction.currentCycle)
    {
        case 0:
        {
            mAddressBus = 0x0000;

            // check condition
            const uint8_t operandCode = (mCurrentInstruction.instructionCode >> 3) & 0b111;
            mCurrentInstruction.conditionMet = mRegisters.checkFlagCondition(static_cast<Registers::FlagCondition>(operandCode));

            if (mCurrentInstruction.conditionMet)
            {
                mCurrentInstruction.instructionCycles = 5;
            }

            break;
        }
        case 1:
        {
            if (mCurrentInstruction.conditionMet == false)
            {
                // nothing to do
                break;
            }

            mAddressBus = mRegisters.stackPointer();
            const uint8_t lowByte = mMemoryManager.getMemoryAtAddress(mAddressBus);
            mCurrentInstruction.temporalData.push_back(lowByte);

            increaseAndStoreStackPointer();
            break;
        }
        case 2:
        {
            mAddressBus = mRegisters.stackPointer();
            const uint8_t highByte = mMemoryManager.getMemoryAtAddress(mAddressBus);
            mCurrentInstruction.temporalData.push_back(highByte);

            increaseAndStoreStackPointer();
            break;
        }
        case 3:
        {
            mAddressBus = 0x0000;

            const std::vector<uint8_t>& tempData = mCurrentInstruction.temporalData;
            const uint16_t newProgramCounter = tempData.at(0) + (tempData.at(1) << 8);

            mRegisters.setProgramCounter(newProgramCounter);
            break;
        }
        default:
        {
            // nothing left to do
            break; 
        }
    }
}