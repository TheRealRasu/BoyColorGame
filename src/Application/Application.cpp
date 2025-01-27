#include "Application.h"

#include "../Hardware/MemoryManager.h"
#include "../Hardware/CPU/Idu.h"

Application::Application()
{
    mIdu = std::make_unique<Idu>();
    mMemoryManager = std::make_unique<MemoryManager>();
}

Application::~Application()
{}

void Application::processInput()
{
    [[maybe_unused]] const uint8_t currentInstruction = mMemoryManager->getMemoryAtAddress(mProgramCounter);

    // TODO implement opcode handling
}