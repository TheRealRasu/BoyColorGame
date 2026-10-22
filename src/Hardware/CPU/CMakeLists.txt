cmake_minimum_required(VERSION 3.7)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(Alu
    ALU/Alu.cpp
    ALU/Alu.h
)

add_library(ControlUnit
    ControlUnit/ControlUnit.cpp
    ControlUnit/ControlUnit.h
)

add_library(CpuCore
    CpuCore/CpuCore.cpp
    CpuCore/CpuCore.h
)

add_library(Idu
    IDU/Idu.cpp
    IDU/Idu.h
)

add_library(Registers
    Registers/Registers.cpp
    Registers/Registers.h
)

target_link_libraries(CpuCore
    Registers
    Idu
    Alu
    ControlUnit
)
