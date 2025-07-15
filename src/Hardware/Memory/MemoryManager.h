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
    std::vector<uint8_t> mWorkRam;
    std::vector<uint8_t> mVideoRam;
};
