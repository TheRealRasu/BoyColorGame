#pragma once

#include <cstdint>
#include <memory>

class CpuCore;

class Application
{
public:
    Application();
    ~Application();

    void processInput();
    
protected:
    std::unique_ptr<CpuCore> mCpuCore;    

};