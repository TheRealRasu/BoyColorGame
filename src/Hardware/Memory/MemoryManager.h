#pragma once

#include <cstdint>
#include <vector>

class MemoryManager
{
public:
    MemoryManager();
    ~MemoryManager();

    uint8_t getMemoryAtAddress(const uint16_t address) const;
    void writeToMemoryAddress(const uint16_t address, const uint8_t value);

    void resetMemory();

protected:
    std::vector<uint8_t> mHighRam; // 127 B of RAM, directly connected to the CPU
    std::vector<uint8_t> mVideoRam; // 16 KB, directly connected to the CPU (for GBC)
    std::vector<uint8_t> mWorkRam; // 32 KB
};
