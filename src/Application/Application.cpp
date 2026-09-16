#include "Application.h"

#include "../Hardware/CPU/CpuCore/CpuCore.h"
#include "../Hardware/Memory/MemoryManager.h"

Application::Application()
{
    mCpuCore = std::make_unique<CpuCore>();
    mMemoryManager = std::make_unique<MemoryManager>();
}

Application::~Application()
{}

void Application::loop()
{
    mCpuCore->handleCurrentInstruction();
}

void Application::processInput()
{
    // TODO implement opcode handling
}

void Application::loadRom(const std::string& fileName)
{
    // TODO actually load
    while (mTerminate == false)
    {
        loop();
    }
}

void Application::resetSystem()
{
    mMemoryManager->resetMemory();
    mCpuCore->reset();
}