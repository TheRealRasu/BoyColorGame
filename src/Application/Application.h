#pragma once

#include <cstdint>

class Application
{
    public:
        Application();
        ~Application();

        void processInput();
        
    protected:

    uint16_t mProgramCounter {};
};