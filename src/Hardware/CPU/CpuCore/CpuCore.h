#pragma once

#include <cstdint>
#include <memory>
#include <vector>

#include "../../Memory/MemoryManager.h"

#include "../ALU/Alu.h"
#include "../ControlUnit/ControlUnit.h"
#include "../IDU/Idu.h"
#include "../Registers/Registers.h"

class CpuCore
{
public:
    CpuCore();
    ~CpuCore() = default;

    struct Instruction
    {
        uint8_t instructionCycles {};
        uint8_t currentCycle {};

        std::vector<uint8_t> temporalData {};

        bool conditionMet {};
    };

    void loadNewInstruction();
    void handleCurrentInstruction();

private:
    void executeInstruction();
    
    void handleZeroZeroInstructionBlock();
    void handleZeroOneInstructionBlock();
    void handleOneZeroInstructionBlock();
    void handleOneOneInstructionBlock();

    // convenience methods
    Registers::BigRegisterIdentifier instructionToBigRegisterValue() const;

    void increaseAndStoreProgramCounter();
    void decreaseAndStoreProgramCounter();

    void increaseAndStoreStackPointer();
    void decreaseAndStoreStackPointer();

    // methods for instructions
    void setFlagsAfterArithmeticOperation(Alu::AluOperationType operation, uint8_t result);
    void unconditionalFunctionCall();
    void conditionalFunctionCall();
    void callAddress();
    void conditionalReturnFromFunction();
    void handleCbInstruction();

    Alu mAlu;
    ControlUnit mControlUnit;
    Registers mRegisters;
    Idu mIdu;
    MemoryManager mMemoryManager;

    Instruction mCurrentInstruction {};

    uint8_t mDataBus {};
    uint16_t mAddressBus {};

};