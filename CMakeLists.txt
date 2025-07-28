cmake_minimum_required(VERSION 3.7)

project(BoyColorGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)

add_library(Application src/Application/Application.cpp src/Application/Application.h src/Application/ApplicationDefines.h)

add_library(Display src/Display/DisplayManager.cpp src/Display/DisplayManager.h)

add_library(MemoryManager src/Hardware/Memory/MemoryManager.cpp src/Hardware/Memory/MemoryManager.h src/Hardware/Memory/MemoryDefines.h)
add_library(Registers src/Hardware/CPU/Registers/Registers.cpp src/Hardware/CPU/Registers/Registers.h)
add_library(ControlUnit src/Hardware/CPU/ControlUnit/ControlUnit.cpp src/Hardware/CPU/ControlUnit/ControlUnit.h)
add_library(Idu src/Hardware/CPU/IDU/Idu.cpp src/Hardware/CPU/IDU/Idu.h)
add_library(Alu src/Hardware/CPU/ALU/Alu.cpp src/Hardware/CPU/ALU/Alu.h)

add_library(CpuCore src/Hardware/CPU/CpuCore/CpuCore.cpp src/Hardware/CPU/CpuCore/CpuCore.h)
target_link_libraries(Display SDL2::SDL2)
target_link_libraries(Application SDL2::SDL2)
target_link_libraries(CpuCore Registers Idu Alu ControlUnit MemoryManager)
target_link_libraries(${PROJECT_NAME} Application Display CpuCore)

include_directories(${PROJECT_NAME} ${SDL2_LIBRARIES})