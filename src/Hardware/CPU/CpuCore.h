#pragma once

#include <cstdint>
#include <memory>

#include "../Memory/MemoryManager.h"

#include "ALU/Alu.h"
#include "ControlUnit/ControlUnit.h"
#include "IDU/Idu.h"
#include "Registers/Registers.h"

class CpuCore
{
public:
    CpuCore();
    ~CpuCore() = default;

    struct Instruction
    {
        uint8_t instructionCode {};
        uint8_t instructionCycles {};
        uint8_t currentCycle {};
    };

    enum class AluOperationType
    {
        add = 0,
        add_plus_carry = 1,
        compare = 2,
        complement_accumulator = 3,
        complement_carry_flag = 4,
        decimal_adjust_accumulator = 5,
        increment = 6,
        logical_and = 7,
        logical_or = 8,
        logical_xor = 9,
        set_carry_flag = 10,
        subtract = 11,
        subtract_plus_carry = 12,
    };

    bool handleCurrentInstruction();
    void decodeNewInstruction(uint8_t newInstruction);
    bool checkInstructionCondition(uint8_t instruction);

private:
    void executeInstruction();

    // convenience methods
    void increaseAndStoreProgramCounter();
    
    // methods for instructions
    void registerAddition();
//    void registerSubtraction();
//    void registerSubtractionWithCarry();
//    void registerComparison();
//    void registerIncrementation();
//    void registerDecrementation();

    Alu mAlu;
    ControlUnit mControlUnit;
    Idu mIdu;
    Registers mRegisters;
    MemoryManager mMemoryManager;

    Instruction mCurrentInstruction {};

    uint8_t mDataBus {};
    uint16_t mAddressBus {};

};