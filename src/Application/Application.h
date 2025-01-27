#pragma once

#include <cstdint>
#include <memory>

class MemoryManager;

class Application
{
    public:
        Application();
        ~Application();

        void processInput();
        
    protected:

        std::unique_ptr<MemoryManager> mMemoryManager;

        uint16_t mProgramCounter {};

        struct __attribute__ ((packed)) Instruction
        {
            uint8_t opCode {};
            uint8_t currentStep {};
        }
};