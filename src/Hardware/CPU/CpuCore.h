#pragma once

#include <cstdint>
#include <memory>
#include <vector>

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

        std::vector<uint8_t> temporalData {};

        bool conditionMet {};
    };

    enum class AluOperationType : uint8_t
    {
        add = 0b000,
        add_plus_carry = 0b001,
        subtract = 0b010,
        subtract_plus_carry = 0b011,
        logical_and = 0b100,
        logical_xor = 0b101,
        logical_or = 0b110,
        compare = 0b111
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
    Registers::BigRegisterIdentifier operandToBigRegister(const uint8_t operand) const;

    void increaseAndStoreProgramCounter();
    void decreaseAndStoreProgramCounter();

    void increaseAndStoreStackPointer();
    void decreaseAndStoreStackPointer();

    // methods for instructions
    void setFlagsAfterArithmeticOperation(uint8_t operationType, uint8_t result);
    void unconditionalFunctionCall();

    Alu mAlu;
    ControlUnit mControlUnit;
    Idu mIdu;
    Registers mRegisters;
    MemoryManager mMemoryManager;

    Instruction mCurrentInstruction {};

    uint8_t mDataBus {};
    uint16_t mAddressBus {};

};