#include "Application.h"

#include "../Hardware/MemoryManager.h"
#include "../Hardware/CPU/CpuCore.h"

Application::Application()
{
    mCpuCore = std::make_unique<CpuCore>();
}

Application::~Application()
{}

void Application::processInput()
{
    

    // TODO implement opcode handling
}