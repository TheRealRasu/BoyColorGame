cmake_minimum_required(VERSION 3.7)
cmake_policy(SET CMP0079 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

add_subdirectory(CPU)

add_library(Timers
    Timers/Timers.cpp
    Timers/Timers.h
)

add_library(Memory
    Memory/MemoryDefines.h
    Memory/MemoryManager.cpp
    Memory/MemoryManager.h
    Memory/RomMemory.cpp
    Memory/RomMemory.h
)

target_link_libraries(CpuCore
    Memory
    Timers
)
