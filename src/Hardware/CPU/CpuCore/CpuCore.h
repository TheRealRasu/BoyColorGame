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
    // methods for instructions
    void executeInstruction();
    
    void handleZeroZeroInstructionBlock();
    void handleZeroOneInstructionBlock();
    void handleOneZeroInstructionBlock();
    void handleOneOneInstructionBlock();

    void unconditionalFunctionCall();
    void conditionalFunctionCall();
    void callAddress();
    void conditionalReturnFromFunction();
    void handleCbInstruction();

    ControlUnit mControlUnit;
    Registers mRegisters;
    Alu mAlu;
    Idu mIdu;
    MemoryManager mMemoryManager;

    Instruction mCurrentInstruction {};

    uint8_t mDataBus {};
    uint16_t mAddressBus {};

};