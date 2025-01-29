#pragma once

#include <cstdint>
#include <memory>

class CpuCore;
class MemoryManager;

class Application
{
public:
    Application();
    ~Application();

    void cpuLoop();

    void processInput();
    
protected:
    std::unique_ptr<CpuCore> mCpuCore;
    std::unique_ptr<MemoryManager> mMemoryManager;

    uint8_t mCurrentInstruction {};

};