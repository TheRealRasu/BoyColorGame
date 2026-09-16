#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <thread>

class CpuCore;
class MemoryManager;

class Application
{
public:
    Application();
    ~Application();

    void loop();

    void processInput();
    void loadRom(const std::string& fileName);
    
    void resetSystem();
    
protected:
    std::unique_ptr<CpuCore> mCpuCore;
    std::unique_ptr<MemoryManager> mMemoryManager;

    bool mTerminate {};
    std::thread mGameLoopThread;
};