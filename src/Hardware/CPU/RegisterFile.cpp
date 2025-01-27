#include "RegisterFile.h"

RegisterFile::RegisterFile()
{}

RegisterFile::~RegisterFile()
{}

uint8_t* RegisterFile::indexToRegister(uint8_t indexCode) const
{
    const uint8_t sanitizedCode = indexCode & 0b111;

    if (sanitizedCode == 0b000) return &mBRegister;
    if (sanitizedCode == 0b001) return &mCRegister;
}