#include "Application.h"

#include "../Hardware/CPU/CpuCore/CpuCore.h"

Application::Application()
{
    mCpuCore = std::make_unique<CpuCore>();
}

Application::~Application()
{}

void Application::cpuLoop()
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
        cpuLoop();
    }
}