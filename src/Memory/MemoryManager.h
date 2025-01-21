#pragma once

#include <cstdint>
#include <vector>

class MemoryManager
{
    public:
        MemoryManager();
        ~MemoryManager();

    uint8_t getMemoryAtAddress(uint16_t address) const;

    protected:
        std::vector<uint8_t> mRam;
        std::vector<uint8_t> mVram;
};
