#include "MemoryManager.h"

MemoryManager::MemoryManager()
{}

MemoryManager::~MemoryManager()
{}

uint8_t MemoryManager::getMemoryAtAddress(uint16_t address) const
{
    return mRam.at(address);
}