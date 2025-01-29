#pragma once

#include <cstdint>
#include <memory>

#include "ALU/Alu.h"
#include "ControlUnit/ControlUnit.h"
#include "IDU/Idu.h"
#include "Registers/Registers.h"

class CpuCore
{
public:
    CpuCore();
    ~CpuCore() = default;

    bool handleCurrentInstruction();
    void decodeNewInstruction(uint8_t newInstruction);
    bool checkInstructionCondition(uint8_t instruction);

    // getters
    uint16_t programCounter() const;
    
private:
    void executeInstruction();
    
    Alu mAlu;
    ControlUnit mControlUnit;
    Idu mIdu;
    Registers mRegisters;

    struct Instruction
    {
        uint8_t instructionCode {};
        uint8_t instructionCycles {};
        uint8_t currentCycle {};
    };

    Instruction mCurrentInstruction {};

    uint8_t mDataBus {};
    uint16_t mAddressBus {};

    void loadBc();
};