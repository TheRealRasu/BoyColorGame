#include "Application.h"

#include "../Hardware/MemoryManager.h"
#include "../Hardware/CPU/CpuCore.h"

Application::Application()
{
    mCpuCore = std::make_unique<CpuCore>();
    mMemoryManager = std::make_unique<MemoryManager>();
}

Application::~Application()
{}

void Application::cpuLoop()
{
    const bool loadNewInstruction = mCpuCore->handleCurrentInstruction();

    if (loadNewInstruction)
    {
        const uint8_t newInstruction = mMemoryManager->getMemoryAtAddress(mCpuCore->programCounter());
        mCpuCore->decodeNewInstruction(newInstruction);
    }
}

void Application::processInput()
{
    

    // TODO implement opcode handling
}