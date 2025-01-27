#include "Application.h"

#include "../Memory/MemoryManager.h"

Application::Application()
{
    mMemoryManager = std::make_unique<MemoryManager>();
}

Application::~Application()
{}

void Application::processInput()
{
    [[maybe_unused]] const uint8_t currentInstruction = mMemoryManager->getMemoryAtAddress(mProgramCounter);

    // TODO implement opcode handling
}