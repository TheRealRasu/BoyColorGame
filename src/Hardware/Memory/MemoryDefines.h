#pragma once

#include <cstdint>

static constexpr uint16_t romBankSize = 0x4000;
static constexpr uint8_t romBankCount = 128;

static constexpr uint16_t ramBankSize = 0x2000;
static constexpr uint8_t ramBankCount = 4;

static constexpr uint16_t vRamMemoryStart = 0x8000;
static constexpr uint16_t vRamMemoryEnd = 0x97FF;