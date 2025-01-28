#pragma once

#include <cstdint>
#include <memory>

class Alu;
class ControlUnit;
class Idu;
class Registers;

class CpuCore
{
public:
    CpuCore();
    ~CpuCore() = default;

    void decodeNewInstruction(uint8_t newInstruction);
    bool checkInstructionCondition(uint8_t instruction);

    bool handleCurrentInstruction();

    
private:
    void executeInstruction();
    
    std::unique_ptr<Alu> mAlu;
    std::unique_ptr<ControlUnit> mControlUnit;
    std::unique_ptr<Idu> mIdu;
    std::unique_ptr<Registers> mRegisters;

    struct __attribute__ ((packed)) Instruction
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