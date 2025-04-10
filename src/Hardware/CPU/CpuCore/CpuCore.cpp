#include "CpuCore.h"

#include "../OpcodeCycleMap.h"

#include <cstdint>
#include <limits>

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
    mCurrentInstruction.conditionMet = false;
    mCurrentInstruction.temporalData.clear();

    // get opcode cycles. Undefined instructions lock the CPU
    const auto instructionIt = cyclesPerOpcode.find(mDataBus);
    if (instructionIt != cyclesPerOpcode.cend())
    {
        mCurrentInstruction.instructionCycles = cyclesPerOpcode.at(mDataBus);
    }
    else
    {
        mCurrentInstruction.instructionCycles = std::numeric_limits<uint8_t>::max();
        }

    // first cycle is always executed during this method
    mIdu.incrementProgramCounter();
}

void CpuCore::executeInstruction()
{
    const uint8_t operationType = mRegisters.instructionRegister() >> 6;
    if (operationType == 0b00)
    {
        handleZeroZeroInstructionBlock();
    }
    else if (operationType == 0b01)
    {
        handleZeroOneInstructionBlock();
    }
    else if (operationType == 0b10)
    {
        handleOneZeroInstructionBlock();
    }
    else // if (operationType == 0b11)
    {
        handleOneOneInstructionBlock();
    }
}

void CpuCore::handleZeroZeroInstructionBlock()
{
    const uint8_t instructionCode = mRegisters.instructionRegister();
    const uint8_t registerId = (instructionCode >> 3) & 0b111;

    switch (instructionCode & 0b111)
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
                const uint16_t otherRegisterValue = mRegisters.bigRegisterValue(Registers::instructionToBigRegisterId(instructionCode));

                if (mCurrentInstruction.currentCycle == 0)
                {
                    mAddressBus = 0x0000;

                    const uint8_t otherValue = static_cast<uint8_t>(otherRegisterValue & 0xFF);
                    mAlu.addToRegister(static_cast<uint8_t>(Registers::SmallRegisterIdentifier::register_l), otherValue);
                }
                else if (mCurrentInstruction.currentCycle == 1)
                {
                    const uint8_t otherValue = static_cast<uint8_t>(otherRegisterValue >> 8) + !!mRegisters.flagValue(Registers::FlagsPosition::carry_flag);
                    mAlu.addToRegister(static_cast<uint8_t>(Registers::SmallRegisterIdentifier::register_h), otherValue);
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

                    mRegisters.setBigRegister(Registers::instructionToBigRegisterId(instructionCode), newValue);
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
                case 0b100: // 0x22 LD (HL+), A
                case 0b110: // 0x32 LD (HL-), A
                {
                    if (mCurrentInstruction.currentCycle == 0)
                    {
                        mAddressBus = mMemoryManager.getMemoryAtAddress(mRegisters.bigRegisterValue(Registers::instructionToBigRegisterId(instructionCode)));
                        mDataBus = mRegisters.accumulator();

                        mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);

                        if (registerId == 0b100)
                        {
                            mIdu.assignAndIncrementRegister(Registers::instructionToBigRegisterId(instructionCode), mAddressBus);
                        }
                        else if (registerId == 0b110)
                        {
                            mIdu.assignAndDecrementRegister(Registers::instructionToBigRegisterId(instructionCode), mAddressBus);
                        }
                    }

                    return;
                }
                case 0b001: // 0x0A LD A, (BC)
                case 0b011: // 0x1A LD A, (DE)
                case 0b101: // 0x2A LD A, (HL+)
                case 0b111: // 0x3A LD A, (HL-)
                {
                    if (mCurrentInstruction.currentCycle == 0)
                    {
                        uint16_t registerValue = mRegisters.bigRegisterValue(Registers::instructionToBigRegisterId(instructionCode));
                        mAddressBus = mMemoryManager.getMemoryAtAddress(registerValue);
                        mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                        
                        if (registerId == 0b101)
                        {
                            mIdu.incrementRegister(Registers::instructionToBigRegisterId(instructionCode));
                        }
                        else if (registerId == 0b111)
                        {
                            mIdu.decrementRegister(Registers::instructionToBigRegisterId(instructionCode));
                        }
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
                const Registers::BigRegisterIdentifier bigRegister = Registers::instructionToBigRegisterId(instructionCode);
                mAddressBus = mRegisters.bigRegisterValue(bigRegister);

                if (registerId & 0b1)
                {
                    mIdu.incrementRegister(bigRegister);
                }
                else
                {
                    mIdu.decrementRegister(bigRegister);
                }
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
                    mAlu.incrementValue(mDataBus);
                    mMemoryManager.writeToMemoryAddress(mAddressBus, mAlu.memory());
                }

                return;
            }

            if (mCurrentInstruction.currentCycle == 0)
            {
                mAlu.incrementRegister(registerId);
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
                    mAlu.decrementValue(mDataBus);
                    mMemoryManager.writeToMemoryAddress(mAddressBus, mAlu.memory());
                }

                return;
            }
            
            if (mCurrentInstruction.currentCycle == 0)
            {
                mAlu.decrementRegister(registerId);
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
                mAlu.loadValueIntoRegister(registerId, mDataBus);
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
                    const bool rotateRight = registerId & 0b1;
                    const bool circular = ((registerId >> 1) & 0b1) == false;

                    if (rotateRight)
                    {
                        mAlu.rotateRight(static_cast<uint8_t>(Registers::SmallRegisterIdentifier::register_acc), circular);
                    }
                    else
                    {
                        mAlu.rotateLeft(static_cast<uint8_t>(Registers::SmallRegisterIdentifier::register_acc), circular);
                    }
                    return;
                }
                case 0b100: // DAA 
                {
                    mAlu.decimalAdjustAccumulator();
                    return;
                }
                case 0b101: // CPL
                {
                    mAlu.flipRegister(static_cast<uint8_t>(Registers::SmallRegisterIdentifier::register_acc));
                    return;
                }
                case 0b110: // set carry flag
                {
                    mAlu.setCarryFlag();
                    return;
                }
                case 0b111: // complement carry flag
                {
                    mAlu.complementCarryFlag();
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

    const uint8_t firstRegister = (instructionCode >> 3) & 0b111;
    const uint8_t secondRegister = instructionCode & 0b111;

    if (firstRegister == 0b110 && secondRegister == 0b110) // special case: 0x76 HALT
    {
        mRegisters.setInterruptEnable(0);
        return;
    }

    if (firstRegister == 0b110) // 0x70 0x71 0x72 0x73 0x74 0x75 0x77 special case: LD (HL), r
    {
        if (mCurrentInstruction.currentCycle == 0)
        {
            mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
            mDataBus = mRegisters.smallRegisterValue(secondRegister);

            mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);
        }

        return;
    }

    if (secondRegister == 0b110) // 0x46 0x56 0x66 0x4E 0x5E 0x6E 0x7E special case: LD r, (HL)
    {
        if (mCurrentInstruction.currentCycle == 0)
        {
            mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
            mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
        }
        else if (mCurrentInstruction.currentCycle == 1)
        {
            mAlu.loadValueIntoRegister(firstRegister, mDataBus);
        }
            
        return;
    }

    // if neither of the operands is equal to 0b110, other operations are simple one-cycle 'load X into Y' (LD n, n) instructions
    // 0x40 0x41 0x42 0x43 0x44 0x45 0x47 0x48 0x49 0x4A 0x4B 0x4C 0x4D 0x4F
    // 0x50 0x51 0x52 0x53 0x54 0x55 0x57 0x58 0x59 0x5A 0x5B 0x5C 0x5D 0x5F
    // 0x60 0x61 0x62 0x63 0x64 0x65 0x67 0x68 0x69 0x6A 0x6B 0x6C 0x6D 0x6F
    // 0x70 0x71 0x72 0x73 0x74 0x75 0x77 0x78 0x79 0x7A 0x7B 0x7C 0x7D 0x7F
    mAlu.loadRegisterIntoRegister(firstRegister, secondRegister);
}

void CpuCore::handleOneZeroInstructionBlock() // DONE
{
    const uint8_t instructionCode = mRegisters.instructionRegister();

    // Arithmetic operations with the accumulator register
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
            mAlu.arithmeticAccumulatorOperation(mDataBus, operation);
        }
        
        return;
    }

    // other instructions:
    // 0x80 0x81 0x82 0x83 0x84 0x85 0x87 0x88 0x89 0x8A 0x8B 0x8C 0x8D 0x8F
    // 0x90 0x91 0x92 0x93 0x94 0x95 0x97 0x98 0x99 0x9A 0x9B 0x9C 0x9D 0x9F
    // 0xA0 0xA1 0xA2 0xA3 0xA4 0xA5 0xA7 0xA8 0xA9 0xAA 0xAB 0xAC 0xAD 0xAF
    // 0xB0 0xB1 0xB2 0xB3 0xB4 0xB5 0xB7 0xB8 0xB9 0xBA 0xBB 0xBC 0xBD 0xBF
    const uint8_t secondRegister = mRegisters.smallRegisterValue(registerOperand);
    mAlu.arithmeticAccumulatorOperation(secondRegister, operation);

    return;
}

void CpuCore::handleOneOneInstructionBlock()
{
    const uint8_t instructionCode = mRegisters.instructionRegister();
    const uint8_t firstOperand = (instructionCode >> 3) & 0b111;

    switch (instructionCode & 0b111)
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
                if (mCurrentInstruction.currentCycle == 0)
                {
                    mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
                    mIdu.incrementProgramCounter();
                }
                else if (mCurrentInstruction.currentCycle == 1)
                {
                    mAddressBus = 0x0000;

                    mDataBus += (mRegisters.stackPointer() & 0xFF);

                    mRegisters.setFlagValue(Registers::FlagsPosition::zero_flag, false);
                    /*
                    mRegisters.setFlagValue(Registers::FlagsPosition::subtraction_flag, false);
                    mRegisters.setFlagValue(Registers::FlagsPosition::half_carry_flag, (mDataBus >> 3 & 0b1));
                    mRegisters.setFlagValue(Registers::FlagsPosition::carry_flag, (mDataBus >> 7 & 0b1));
*/
                    mCurrentInstruction.temporalData.push_back(mDataBus);
                }
                else if (mCurrentInstruction.currentCycle == 2)
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
                    
                    mRegisters.setBigRegister(Registers::instructionToBigRegisterId(instructionCode), newValue);
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
        case 0b011: // 0xC3 0xCB 0xF3 0xFB
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
            else if (firstOperand == 0b001) // 0xCB CB
            {
                handleCbInstruction();
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
                            mDataBus = mRegisters.bigRegisterValue(Registers::instructionToBigRegisterId(instructionCode)) >> 8;

                            mMemoryManager.writeToMemoryAddress(mAddressBus, mDataBus);
                            mIdu.decrementStackPointer();
                            return;
                        }
                        case 2:
                        {
                            mAddressBus = mRegisters.stackPointer();
                            mDataBus = static_cast<uint8_t>(mRegisters.bigRegisterValue(Registers::instructionToBigRegisterId(instructionCode)) & 0xFF);

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
                const Alu::AluOperationType operation = static_cast<Alu::AluOperationType>(firstOperand);
                mAlu.arithmeticAccumulatorOperation(mDataBus, operation);
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
    if (mCurrentInstruction.currentCycle == 0)
    {
        mIdu.incrementProgramCounter();
        return;
    }

    const uint8_t currentInstruction = mRegisters.instructionRegister();

    const uint8_t instructionBlock = (currentInstruction >> 6) & 0b11;
    const uint8_t registerId = currentInstruction & 0b111;
    const uint8_t bitId = (currentInstruction >> 3) & 0b111;

    if (registerId == 0b110) // HL operations
    {
        if (instructionBlock == 0b00)
        {
            // TODO various HL instructions
        }
        else
        {
            if (mCurrentInstruction.currentCycle == 1)
            {
                // (HL) bit test needs three cycles, bit set and bit reset instructions need 4
                mCurrentInstruction.instructionCycles = (instructionBlock == 0b01) ? 3 : 4;

                mAddressBus = mRegisters.bigRegisterValue(Registers::BigRegisterIdentifier::register_hl);
                mDataBus = mMemoryManager.getMemoryAtAddress(mAddressBus);
            }
            else if (mCurrentInstruction.currentCycle == 2)
            {
                mAlu.bitOperation(mDataBus, bitId, static_cast<Alu::BitOperationType>(instructionBlock));

                if (instructionBlock != 0b01) 
                {
                    mMemoryManager.writeToMemoryAddress(mAddressBus, mAlu.memory());
                }
            }
        }
    }
    else if (instructionBlock == 0b00)
    {
        switch (bitId)
        {
            case 0b000: // RLC r
            {
                mAlu.rotateLeft(registerId, true);
                break;
            }
            case 0b001: // RRC r
            {
                mAlu.rotateRight(registerId, true);
                break;
            }
            case 0b010: // RL r
            {
                mAlu.rotateLeft(registerId, false);
                break;
            }
            case 0b011: // RR r
            {
                mAlu.rotateRight(registerId, false);
                break;
            }
            case 0b100: // SLA r
            case 0b101: // SRA r
            case 0b110: // SWAP r
            case 0b111: // SRL r
            {
                // TODO
                break;
            }
        }

        return;
        // TODO various instructions 
    }
    else // neither HL nor 0b00
    {
        if (mCurrentInstruction.currentCycle == 1)
        {
            const uint8_t registerValue = mRegisters.smallRegisterValue(registerId);
            mAlu.bitOperation(registerValue, bitId, static_cast<Alu::BitOperationType>(instructionBlock));
            
            if (instructionBlock != 0b01) 
            {
                mRegisters.setSmallRegister(registerId, mAlu.memory());
            }
        }
    }
}