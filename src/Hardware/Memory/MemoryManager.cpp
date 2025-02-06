#include "MemoryManager.h"

MemoryManager::MemoryManager()
{}

MemoryManager::~MemoryManager()
{}

uint8_t MemoryManager::getMemoryAtAddress(uint16_t address) const
{
    return mWorkRam.at(address);
}

void MemoryManager::writeToMemoryAddress(const uint16_t address, const uint8_t value)
{
    mWorkRam.at(address) = value;
}