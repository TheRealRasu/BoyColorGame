#include "Application.h"

#include "../Hardware/CPU/CpuCore.h"

Application::Application()
{
    mCpuCore = std::make_unique<CpuCore>();
}

Application::~Application()
{}

void Application::cpuLoop()
{
    const bool loadNewInstruction = mCpuCore->handleCurrentInstruction();

    if (loadNewInstruction)
    {
        // mCpuCore->decodeNewInstruction();
    }
}

void Application::processInput()
{
    

    // TODO implement opcode handling
}