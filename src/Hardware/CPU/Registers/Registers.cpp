#include "Registers.h"

Registers::Registers()
{}

Registers::~Registers()
{}

uint16_t Registers::programCounter()
{
    return mProgramCounter;
}

uint8_t Registers::singleRegisterValue(uint8_t identifier) const
{
    switch (identifier)
    {
        // TODO
    }

    return 0u;
}

uint16_t Registers::combinedRegisterValue(uint8_t identifier) const
{
    switch (identifier)
    {
        // TODO
    }

    return 0u;
}

uint8_t& Registers::singleRegister(uint8_t identifier)
{
    switch (identifier)
    {
        // TODO
    }

    return mBRegister;
}

void setSingleRegister(uint8_t identifier, uint8_t /* value */)
{
    switch (identifier)
    {
        // TODO
    }
}
void setCombinedRegister(uint8_t identifier, uint16_t /* value */)
{
    switch (identifier)
    {
        // TODO
    }
}

void Registers::setProgramCounter(uint16_t newValue)
{
    mProgramCounter = newValue;
}