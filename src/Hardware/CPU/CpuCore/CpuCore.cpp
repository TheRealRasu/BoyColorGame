#include "CpuCore.h"

#include "../OpcodeCycleMap.h"

#include <cstdint>

CpuCore::CpuCore()
    : mAlu(mRegisters),
      mIdu(mRegisters)
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
    mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
    
    mRegisters.setInstructionRegister(mDataBus);
    
    // reset Instruction struct
    mCurrentInstruction.currentCycle = 0;
    mCurrentInstruction.instructionCycles = cyclesPerOpcode.at(mDataBus);
    mCurrentInstruction.conditionMet = false;
    mCurrentInstruction.temporalData.clear();

    // first cycle is always executed during this method
    mIdu.incrementProgramCounter();
}

void CpuCore::executeInstruction()
{
    const uint8_t operationType = mRegisters.instructionRegister() >> 6;
    
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
    const uint8_t instructionCode = mRegisters.instructionRegister();

    const uint8_t registerId = (instructionCode >> 3) & 0b111;
    const uint8_t mainOperand = instructionCode & 0b111;

    switch (mainOperand)
    {
        case 0b000: // 0x00, 0x08, 0x10, 0x18, 0x20, 0x28, 0x30, 0x38 TODO
        {
            switch (registerId)
            {
                case 0b000: // 0x00, NOP
                {
                    // No operation
                    return;
                }
                case 0b001: // 0x08 LD (nn), SP
                {
                    if ((mCurrentInstruction.currentCycle == 0) || (mCurrentInstruction.currentCycle == 1))
                    {
                        mAddressBus = mRegisters.programCounter();
                        mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);

                        mCurrentInstruction.temporalData.push_back(mDataBus);
                        mIdu.incrementProgramCounter();
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
                case 0b010: // 0x10 Stop. TODO Consult with CPU documents.
                {
                    return;
                }
                case 0b011: // 0x18 JR e
                {
                    if (mCurrentInstruction.currentCycle == 0)
                    {
                        mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                        mCurrentInstruction.temporalData.push_back(mDataBus);

                        mIdu.incrementProgramCounter();
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
                case 0b100: // 0x20 JR NZ, e
                case 0b101: // 0x28 JR Z, e
                case 0b110: // 0x30 JR NC, e
                case 0b111: // 0x38 JR C, e
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

                        mIdu.incrementProgramCounter();
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
        case 0b001: // 0x01, 0x09, 0x11, 0x19, 0x21, 0x29, 0x31, 0x39
        {
            if (registerId & 0b1) // 0x09 0x19 0x29 0x39 ADD HL, rr
            {
                if (mCurrentInstruction.currentCycle == 0)
                {
                    mAddressBus = 0x0000;

                    const uint8_t lRegister = static_cast<uint8_t>(mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl) & 0xFF);
                    const uint8_t otherRegister = static_cast<uint8_t>(mRegisters.bigRegisterValue(instructionToBigRegisterValue()) & 0xFF);

                    const uint8_t result = lRegister + otherRegister; // TODO move this to ALU
                    setFlagsAfterArithmeticOperation(Alu::AluOperationType::add, result);

                    mRegisters.setSmallRegister(0b101, result);
                }
                else if (mCurrentInstruction.currentCycle == 1)
                {
                    const uint8_t hRegister = static_cast<uint8_t>(mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl) >> 8);
                    const uint8_t otherRegister = static_cast<uint8_t>(mRegisters.bigRegisterValue(instructionToBigRegisterValue()) >> 8);

                    const uint8_t result = hRegister + otherRegister + !!mRegisters.flagValue(Registers::FlagsPosition::carry_flag); // TODO move this to ALU
                    setFlagsAfterArithmeticOperation(Alu::AluOperationType::add, result);

                    mRegisters.setSmallRegister(0b100, result);
                }
            }
            else // 0x01 0x11 0x21 0x31 LD rr, nn
            {
                if (mCurrentInstruction.currentCycle == 0 || mCurrentInstruction.currentCycle == 1)
                {
                    mAddressBus = mRegisters.programCounter();
                    mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);

                    mCurrentInstruction.temporalData.push_back(mDataBus);
                    mIdu.incrementProgramCounter();
                    break;
                }
                else if (mCurrentInstruction.currentCycle == 2)
                {
                    const std::vector<uint8_t>& tempData = mCurrentInstruction.temporalData;
                    const uint16_t newValue = tempData.at(0) + (tempData.at(1) << 8);

                    mRegisters.setBigRegister(instructionToBigRegisterValue(), newValue);
                }
            }
            break;
        }
        case 0b010: // 0x02, 0x0A, 0x12, 0x1A, 0x22, 0x2A, 0x32, 0x3A. Load data
        {
            switch (registerId)
            {
                case 0b000: // 0x02 LD (BC), A
                case 0b010: // 0x12 LD (DE), A
                {
                    if (mCurrentInstruction.currentCycle == 0)
                    {
                        mAddressBus = mMemoryManager.getMemoryAtAddress(mRegisters.bigRegisterValue(instructionToBigRegisterValue()));
                        mDataBus = mRegisters.accumulator();

                        mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);
                    }

                    return;
                }
                case 0b100: // 0x22 LD (HL+), A
                case 0b110: // 0x32 LD (HL-), A
                {
                    if (mCurrentInstruction.currentCycle == 0)
                    {
                        mAddressBus = mMemoryManager.getMemoryAtAddress(mRegisters.bigRegisterValue(instructionToBigRegisterValue()));
                        mDataBus = mRegisters.accumulator();

                        mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);

                        const uint16_t newRegisterValue = mAddressBus;

                        if (registerId == 0b100)
                        {
                            mIdu.increaseValue(mAddressBus);
                        }
                        else if (registerId == 0b110)
                        {
                            mIdu.decreaseValue(mAddressBus);
                        }

                        mRegisters.setBigRegister(instructionToBigRegisterValue(), mIdu.memory());
                    }

                    return;
                }
                case 0b001: // 0x0A LD A, (BC)
                case 0b011: // 0x1A LD A, (DE)
                {
                    if (mCurrentInstruction.currentCycle == 0)
                    {
                        mAddressBus = mMemoryManager.getMemoryAtAddress(mRegisters.bigRegisterValue(instructionToBigRegisterValue()));
                        mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                    }
                    else if (mCurrentInstruction.currentCycle == 1)
                    {
                        mRegisters.setAccumulator(mDataBus);
                    }

                    return;
                }
                case 0b101: // 0x2A LD A, (HL+)
                case 0b111: // 0x3A LD A, (HL-)
                {
                    if (mCurrentInstruction.currentCycle == 0)
                    {
                        uint16_t hlValue = mRegisters.bigRegisterValue(instructionToBigRegisterValue());
                        mAddressBus = mMemoryManager.getMemoryAtAddress(hlValue);
                        mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);

                        if (registerId == 0b101)
                        {
                            mIdu.increaseValue(hlValue);
                        }
                        else if (registerId == 0b111)
                        {
                            mIdu.decreaseValue(hlValue);
                        }

                        mRegisters.setBigRegister(instructionToBigRegisterValue(), mIdu.memory());
                    }
                    else if (mCurrentInstruction.currentCycle == 1)
                    {
                        mRegisters.setAccumulator(mDataBus);
                    }

                    return;
                }
            }

            return;
        }
        case 0b011: // 0x03, 0x0B, 0x13, 0x1B, 0x23, 0x2B, 0x33, 0x3B. INC/DEC rr
        {
            if (mCurrentInstruction.currentCycle == 0)
            {
                mAddressBus = mRegisters.bigRegisterValue(instructionToBigRegisterValue());

                if (registerId & 0b1)
                {
                    mIdu.increaseValue(mAddressBus);
                }
                else
                {
                    mIdu.decreaseValue(mAddressBus);
                }

                const uint16_t addressBus = mIdu.memory();

                mRegisters.setBigRegister(instructionToBigRegisterValue(), addressBus);
            }

            break;
        }
        case 0b100: // 0x04, 0x0C, 0x14, 0x1C, 0x24, 0x2C, 0x34, 0x3C. increment register
        {
            if (registerId == 0b110) // 0x34 special case: HL register
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

            if (mCurrentInstruction.currentCycle == 0)
            {
                uint8_t newRegisterValue = mRegisters.smallRegisterValue(registerId);
                mAlu.incrementRegister(newRegisterValue);
                mRegisters.setSmallRegister(registerId, newRegisterValue);

                mRegisters.setFlagValue(Registers::FlagsPosition::zero_flag, newRegisterValue == 0);
                mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
                mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (newRegisterValue >> 3) & 0b1);
                return;
            }
            break;
        }
        case 0b101: // 0x05, 0x0D, 0x15, 0x1D, 0x25, 0x2D, 0x35, 0x3D. decrement register
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
            
            if (mCurrentInstruction.currentCycle == 0)
            {
                uint8_t newRegisterValue = mRegisters.smallRegisterValue(registerId);
                mAlu.decrementRegister(newRegisterValue);
                mRegisters.setSmallRegister(registerId, newRegisterValue);

                mRegisters.setFlagValue(Registers::FlagsPosition::zero_flag, newRegisterValue == 0);
                mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, true);
                mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (newRegisterValue >> 3) & 0b1);
                break;
            }
        }
        case 0b110: // 0x06, 0x0E, 0x16, 0x1E, 0x26, 0x2E, 0x36, 0x3E. load incoming data to register
        {
            if (registerId == 0b110) // special case: HL register
            {
                if (mCurrentInstruction.currentCycle == 0)
                {
                    mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                    mIdu.incrementProgramCounter();
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
                mIdu.incrementProgramCounter();
            }
            else if (mCurrentInstruction.currentCycle == 1)
            {
                uint8_t registerValue = mRegisters.smallRegisterValue(registerId);
                mAlu.assignRegisterValue(mDataBus, registerValue);

                mRegisters.setSmallRegister(registerId, registerValue);
            }

            break;
        }
        case 0b111: // 0x07, 0x0F, 0x17, 0x1F, 0x27, 0x2F, 0x37, 0x3F. one TODO
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
                    mRegisters.setAccumulator(mAlu.memory());

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
    const uint8_t instructionCode = mRegisters.instructionRegister();

    const uint8_t firstOperand = (instructionCode >> 3) & 0b111;
    const uint8_t secondOperand = instructionCode & 0b111;

    if (firstOperand == 0b110 && secondOperand == 0b110) // special case: 0x76 HALT
    {
        mRegisters.setInterruptEnable(0);
        return;
    }

    if (firstOperand == 0b110) // 0x70 0x71 0x72 0x73 0x74 0x75 0x77 special case: LD (HL), n
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

    if (secondOperand == 0b110) // 0x46 0x56 0x66 0x4E 0x5E 0x6E 0x7E special case: LD n, (HL)
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

    // if neither of the operands is equal to 0b110, other operations are simple one-cycle 'load X into Y' (LD n, n) instructions
    // 0x40 0x41 0x42 0x43 0x44 0x45 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4F
    // 0x50 0x51 0x52 0x53 0x54 0x55 0x57 0x58 0x59 0x5A 0x5B 0x5C 0x5D 0x5F
    // 0x60 0x61 0x62 0x63 0x64 0x65 0x67 0x68 0x69 0x6A 0x6B 0x6C 0x6D 0x6F
    // 0x70 0x71 0x72 0x73 0x74 0x75 0x77 0x78 0x79 0x7A 0x7B 0x7C 0x7D 0x7F

    uint8_t registerValue = mRegisters.smallRegisterValue(firstOperand);
    const uint8_t srcRegister = mRegisters.smallRegisterValue(secondOperand);

    mAlu.assignRegisterValue(srcRegister, registerValue);
    mRegisters.setSmallRegister(firstOperand, registerValue);

    return;
}

void CpuCore::handleOneZeroInstructionBlock() // DONE
{
    const uint8_t instructionCode = mRegisters.instructionRegister();

    // Arithmetic operations with the accumulator register
    const uint8_t accumulatorValue = mRegisters.accumulator();
    const Alu::AluOperationType operation = static_cast<Alu::AluOperationType>((instructionCode >> 3) & 0b111);
    const uint8_t registerOperand = instructionCode & 0b111;

    if (registerOperand == 0b110) // special case: 0x86 0x96 0xA6 0xB6 0x8E 0x9E 0xAE 0xBE HL register instructions
    {
        if (mCurrentInstruction.currentCycle == 0)
        {
            mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
            mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
        }
        else if (mCurrentInstruction.currentCycle == 1)
        {
            mAlu.arithmeticOperation(accumulatorValue, mDataBus, operation);
            
            if (operation != Alu::AluOperationType::compare)
            {
                mRegisters.setAccumulator(mAlu.memory());
            }

            setFlagsAfterArithmeticOperation(operation, mAlu.memory());
        }
        
        return;
    }

    // other instructions:
    // 0x80 0x81 0x82 0x83 0x84 0x85 0x87 0x88 0x89 0x8A 0x8B 0x8C 0x8D 0x8F
    // 0x90 0x91 0x92 0x93 0x94 0x95 0x97 0x98 0x99 0x9A 0x9B 0x9C 0x9D 0x9F
    // 0xA0 0xA1 0xA2 0xA3 0xA4 0xA5 0xA7 0xA8 0xA9 0xAA 0xAB 0xAC 0xAD 0xAF
    // 0xB0 0xB1 0xB2 0xB3 0xB4 0xB5 0xB7 0xB8 0xB9 0xBA 0xBB 0xBC 0xBD 0xBF

    const uint8_t secondRegister = mRegisters.smallRegisterValue(registerOperand);

    bool additionalFlag {};
    if ((operation == Alu::AluOperationType::add_plus_carry) || (operation == Alu::AluOperationType::subtract_plus_carry))
    {
        additionalFlag = mRegisters.flagValue(Registers::FlagsPosition::carry_flag);
    }

    mAlu.arithmeticOperation(accumulatorValue, secondRegister, operation, additionalFlag);
    
    if (operation != Alu::AluOperationType::compare)
    {
        mRegisters.setAccumulator(mAlu.memory());
    }

    setFlagsAfterArithmeticOperation(operation, mAlu.memory());
    return;
}

void CpuCore::handleOneOneInstructionBlock()
{
    const uint8_t instructionCode = mRegisters.instructionRegister();

    const uint8_t firstOperand = (instructionCode >> 3) & 0b111;
    const uint8_t mainOperand = instructionCode & 0b111;

    switch (mainOperand)
    {
        case 0b000: // various; TODO
        {
            if (firstOperand == 0b100) // 0xE0 LDH (n)
            {
                switch (mCurrentInstruction.currentCycle)
                {
                    case 0:
                    {
                        const uint8_t lowByte = mMemoryManager.getMemoryAtAddress(mAddressBus);
                        mCurrentInstruction.temporalData.push_back(lowByte);

                        mIdu.incrementProgramCounter();
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
            else if (firstOperand == 0b101) // 0xE8 TODO ADD SP, e
            {
                if (const uint8_t curr = mCurrentInstruction.currentCycle; curr == 0)
                {
                    mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                    mIdu.incrementProgramCounter();
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
            else if (firstOperand == 0b110) // 0xF0 LDH A, (n)
            {
                switch (mCurrentInstruction.currentCycle)
                {
                    case 0:
                    {
                        mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                        mIdu.incrementProgramCounter();
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
            else if (firstOperand == 0b111) // 0xF8 LD HL, SP+e TODO
            {
                // TODO LD HL, SP+e
            }
            else // 0xC0 0xC8 0xD0 0xD8 RET condition
            {
                conditionalReturnFromFunction();
            }
            break;
        }
        case 0b001: // 0xC1 0xD1 0xE1 0xF1 'POP rr' 0xC9 'RET' 0xD9 'RETI' 0xE9 'JP HL' 0xF9 'LD SP, HL'
        {
            if (firstOperand == 0b001 || firstOperand == 0b011) // 0xC9 RET and 0xD9 RETI
            {
                switch (mCurrentInstruction.currentCycle)
                {
                    case 0:
                    case 1:
                    {
                        mAddressBus = mRegisters.stackPointer();
                        mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);

                        mCurrentInstruction.temporalData.push_back(mDataBus);

                        mIdu.incrementStackPointer();
                        break;
                    }
                    case 2:
                    {
                        mAddressBus = 0x0000;
                        const std::vector<uint8_t>& temporalData = mCurrentInstruction.temporalData;
                        const uint16_t newValue = temporalData.at(0) + (temporalData.at(1) << 8);
                        
                        mRegisters.setProgramCounter(newValue);

                        if (firstOperand == 0b011)
                        {
                            mRegisters.setInterruptEnable(1u);
                        }

                        break;
                    }
                    default:
                    {
                        // do nothing; we're done here
                        break;
                    }
                }

                return;
            }
            else if (firstOperand == 0b101) // 0xE9 JP HL
            {
                mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
                mRegisters.setProgramCounter(mAddressBus);
                return;
            }
            else if (firstOperand == 0b111) // 0xF9 LD SP, HL
            {
                mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
                mRegisters.setStackPointer(mAddressBus);
                return;
            }

            // 0xC1 0xD1 0xE1 0xF1 pop from stack
            switch (mCurrentInstruction.currentCycle)
            {
                case 0:
                case 1:
                {
                    mAddressBus = mRegisters.stackPointer();
                    mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);

                    mCurrentInstruction.temporalData.push_back(mDataBus);

                    mIdu.incrementStackPointer();
                    break;
                }
                case 2:
                {
                    const std::vector<uint8_t>& temporalData = mCurrentInstruction.temporalData;
                    const uint16_t newValue = temporalData.at(0) + (temporalData.at(1) << 8);
                    
                    mRegisters.setBigRegister(instructionToBigRegisterValue(), newValue);
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
        case 0b010: // 0xC2 0xCA 0xD2 0xDA 'JP cc, nn' 0xE2 0xF2 'LDH n, n' 0xEA 0xFA 'LD n, nn'
        {
            if ((firstOperand >> 2 & 0b1) == false) // 0xC2 0xD2 0xCA 0xDA 'JP cc, nn'
            {
                if (mCurrentInstruction.currentCycle == 0 || mCurrentInstruction.currentCycle == 1)
                {
                    mAddressBus = mRegisters.programCounter();
                    mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                    mCurrentInstruction.temporalData.push_back(mDataBus);

                    mIdu.incrementProgramCounter();

                    if (mCurrentInstruction.currentCycle == 1)
                    {
                        const bool conditionMet = mRegisters.checkFlagCondition(static_cast<Registers::FlagCondition>(firstOperand & 0b11));
                        mCurrentInstruction.conditionMet = conditionMet;
                        if (conditionMet)
                        {
                            mCurrentInstruction.instructionCycles = 4;
                        }
                    }
                    if (mCurrentInstruction.currentCycle == 2)
                    {
                        if (mCurrentInstruction.conditionMet)
                        {
                            mAddressBus = 0x0000;

                            const std::vector<uint8_t>& tempData = mCurrentInstruction.temporalData;
                            const uint16_t newPc = tempData.at(0) + (tempData.at(1) << 8);
                            mRegisters.setProgramCounter(newPc);
                        }
                    }
                }
            }
            else if (firstOperand == 0b100) // 0xE2 'LDH (C), A'
            {
                if (mCurrentInstruction.currentCycle == 0)
                {
                    const uint8_t cRegister = mRegisters.smallRegisterValue(0b001);
                    mAddressBus = 0xFF00 + cRegister;

                    mDataBus = mRegisters.accumulator();

                    mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);
                }
                else if (mCurrentInstruction.currentCycle == 1)
                {
                    // nothing left to do
                }
            }
            else if (firstOperand == 0b110) // 0xF2 'LDH A, (C)'
            {
                if (mCurrentInstruction.currentCycle == 0)
                {
                    const uint8_t cRegister = mRegisters.smallRegisterValue(0b001);
                    mAddressBus = 0xFF00 + cRegister;

                    mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                }
                else if (mCurrentInstruction.currentCycle == 1)
                {
                    mRegisters.setAccumulator(mDataBus);
                }
            }
            else if (firstOperand == 0b101) // 0xEA 'LD (nn), A'
            {
                if (mCurrentInstruction.currentCycle == 0 || mCurrentInstruction.currentCycle == 1)
                {
                    mAddressBus = mRegisters.programCounter();
                    mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                    mCurrentInstruction.temporalData.push_back(mDataBus);

                    mIdu.incrementProgramCounter();
                }
                else if (mCurrentInstruction.currentCycle == 2)
                {
                    const std::vector<uint8_t>& tempData = mCurrentInstruction.temporalData;
                    mAddressBus = tempData.at(0) + (tempData.at(1) << 8);

                    mDataBus = mRegisters.accumulator();
                    mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);
                }
                else if (mCurrentInstruction.currentCycle == 3)
                {
                    // nothing left to do
                }
            }
            else if (firstOperand == 0b111) // 0xFA 'LD A, (nn)
            {
                if (mCurrentInstruction.currentCycle == 0 || mCurrentInstruction.currentCycle == 1)
                {
                    mAddressBus = mRegisters.programCounter();
                    mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                    mCurrentInstruction.temporalData.push_back(mDataBus);

                    mIdu.incrementProgramCounter();
                }
                else if (mCurrentInstruction.currentCycle == 2)
                {
                    const std::vector<uint8_t>& tempData = mCurrentInstruction.temporalData;

                    mAddressBus = tempData.at(0) + (tempData.at(1) << 8);
                    mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                }
                else if (mCurrentInstruction.currentCycle == 3)
                {
                    mRegisters.setAccumulator(mDataBus);
                }
            }

            return;
        }
        case 0b011: // 0xC3 0xCB 0xF3 0xFB TODO
        {
            if (firstOperand == 0b000) // 0xC3 JP nn
            {
                switch (mCurrentInstruction.currentCycle)
                {
                    case 0:
                    case 1:
                    {
                        mAddressBus = mRegisters.programCounter();
                        mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);

                        mCurrentInstruction.temporalData.push_back(mDataBus);

                        mIdu.incrementProgramCounter();
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
            else if (firstOperand == 0b001) // 0xCB CB op TODO
            {
                if (mCurrentInstruction.currentCycle == 0)
                {
                    mIdu.incrementProgramCounter();
                }
                else if (mCurrentInstruction.currentCycle == 1)
                {
                    handleCbInstruction();
                }
            }
            else if (firstOperand == 0b110) // 0xF3 DI
            {
                mRegisters.setInterruptEnable(0u);
            }
            else if (firstOperand == 0b111) // 0xFB EI
            {
                mRegisters.setInterruptEnable(1u);
            }
            return;
        }
        case 0b100: // 0xC4 0xD4 0xCC 0xDC 'CALL cc, nn'
        {
            if ((firstOperand >> 2) & 0b1) return; // 0xE4 0xEC 0xF4 0xFC undefined actions

            conditionalFunctionCall();
            break;
        }
        case 0b101: // 0xC5 0xCD 0xD5 0xE5 0xF5 'PUSH rr' + 'CALL NN'
        {
            switch (firstOperand)
            {
                case 0b011: // 0xDD
                case 0b101: // 0xED
                case 0b111: // 0xFD
                {
                    // no operation
                    return;
                }
                case 0b001: // 0xCD 'CALL nn'
                {
                    callAddress();
                    break;
                }
                case 0b000: // 0xC5 'PUSH BC'
                case 0b010: // 0xD5 'PUSH DE'
                case 0b100: // 0xE5 'PUSH HL'
                case 0b110: // 0xF5 'PUSH AF'
                {
                    // Push to Stack
                    switch (mCurrentInstruction.currentCycle)
                    {
                        case 0:
                        {
                            mAddressBus = mRegisters.stackPointer();
                            mIdu.decrementStackPointer();
                            return;
                        }
                        case 1:
                        {
                            mAddressBus = mRegisters.stackPointer();
                            mDataBus = mRegisters.bigRegisterValue(instructionToBigRegisterValue()) >> 8;

                            mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);
                            mIdu.decrementStackPointer();
                            return;
                        }
                        case 2:
                        {
                            mAddressBus = mRegisters.stackPointer();
                            mDataBus = static_cast<uint8_t>(mRegisters.bigRegisterValue(instructionToBigRegisterValue()) & 0xFF);

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
        case 0b110: // 0xC6 0xCE 0xD6 0xDE 0xE6 0xEE 0xF6 0xFE 'immediate data operation'
        {
            if (mCurrentInstruction.currentCycle == 0)
            {
                mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                mIdu.incrementProgramCounter();
            }
            else if (mCurrentInstruction.currentCycle == 1)
            {
                const uint8_t accumulatorValue = mRegisters.accumulator();
                const Alu::AluOperationType operation = static_cast<Alu::AluOperationType>(firstOperand);
                mAlu.arithmeticOperation(accumulatorValue, mDataBus, operation);
                
                if (operation != Alu::AluOperationType::compare)
                {
                    mRegisters.setAccumulator(mAlu.memory());
                }

                setFlagsAfterArithmeticOperation(operation, mAlu.memory());
                return;
            }
            break;
        }
        case 0b111: // 0xC7 0xCF 0xD7 0xDF 0xE7 0xEF 0xF7 0xFF 'RST 0xNN'
        {
            unconditionalFunctionCall();
            break;
        }
    }
}

Registers::BigRegisterIdentifier CpuCore::instructionToBigRegisterValue() const
{
    const uint8_t instructionCode = mRegisters.instructionRegister();

    switch (instructionCode >> 4 & 0b11)
    {
        case 0b00: return Registers::BigRegisterIdentifier::register_bc;
        case 0b01: return Registers::BigRegisterIdentifier::register_de;
        case 0b10: return Registers::BigRegisterIdentifier::register_hl;
        case 0b11:
        {
            if (((instructionCode >> 6) & 0b11) == 0b11)
            {
                return Registers::BigRegisterIdentifier::register_af;
            }

            if ((instructionCode & 0b11) == 0b10)
            {
                return Registers::BigRegisterIdentifier::register_hl;
            }           

            return Registers::BigRegisterIdentifier::register_sp;
        }
        default: break;
    }

    return Registers::BigRegisterIdentifier::register_bc;
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
        mIdu.decrementStackPointer();
    }
    else if (mCurrentInstruction.currentCycle == 1)
    {
        mAddressBus = mRegisters.stackPointer();

        mDataBus = (mRegisters.programCounter() >> 8); // most significant byte
        mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);

        mIdu.decrementStackPointer();
    }
    else if (mCurrentInstruction.currentCycle == 2)
    {
        mAddressBus = mRegisters.stackPointer();

        mDataBus = (mRegisters.programCounter() & 0xFF); // least significant byte
        mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);

        const uint16_t newAddress = mRegisters.instructionRegister() & 0x38;
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
            mIdu.incrementProgramCounter();

            break;
        }
        case 1:
        {
            mAddressBus = mRegisters.programCounter();
            mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);

            mCurrentInstruction.temporalData.push_back(mDataBus); // high byte
            mIdu.incrementProgramCounter();

            const uint8_t operandCode = (mRegisters.instructionRegister() >> 3) & 0b111;
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
            mIdu.decrementStackPointer();

            break;
        }
        case 3:
        {
            mAddressBus = mRegisters.stackPointer();
            mDataBus = mRegisters.programCounter() >> 8; // most significant PC byte

            mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);
            mIdu.decrementStackPointer();

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
            mIdu.incrementProgramCounter();
            break;
        }
        case 2:
        {
            mAddressBus = mRegisters.stackPointer();
            mIdu.decrementStackPointer();
            break;
        }
        case 3:
        {
            mAddressBus = mRegisters.stackPointer();
            mDataBus = mRegisters.programCounter() >> 8;

            mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);

            mIdu.decrementStackPointer();
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
            const uint8_t operandCode = (mRegisters.instructionRegister() >> 3) & 0b111;
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

            mIdu.incrementStackPointer();
            break;
        }
        case 2:
        {
            mAddressBus = mRegisters.stackPointer();
            const uint8_t highByte = mMemoryManager.getMemoryAtAddress(mAddressBus);
            mCurrentInstruction.temporalData.push_back(highByte);

            mIdu.incrementStackPointer();
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

void CpuCore::handleCbInstruction()
{
    const uint8_t currentInstruction = mRegisters.instructionRegister();

    const uint8_t instructionBlock = (currentInstruction >> 6) & 0b11;

    const uint8_t registerId = currentInstruction & 0b111;
    const uint8_t bitId = (currentInstruction >> 3) & 0b111;

    if (registerId == 0b110) // HL operations
    {

    }
    else if (instructionBlock == 0b00)
    {
        // TODO various instructions 
    }
    else // neither HL nor 0b00
    {
        if (mCurrentInstruction.currentCycle == 1)
        {
            const uint8_t registerValue = mRegisters.smallRegisterValue(registerId);
            mAlu.bitOperation(registerValue, bitId, static_cast<Alu::BitOperationType>(instructionBlock));
            mRegisters.setSmallRegister(registerId, mAlu.memory());

            if (instructionBlock == 0b01) // test bit
            {
                mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, true);
                mRegisters.setFlagValue(Registers::FlagsPosition::zero_flag, (mAlu.memory() == 0));
                mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
            }
        }
    }
}