cmake_minimum_required(VERSION 3.7)

project(BoyColorGame)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 REQUIRED)

add_executable(${PROJECT_NAME} src/main.cpp)

add_library(Application src/Application/Application.cpp src/Application/Application.h src/Application/ApplicationDefines.h)
#add_library(DisplayHandler src/Application/DisplayHandler.cpp src/Application/DisplayHandler.h src/Application/ApplicationDefines.h)
add_library(Alu src/Hardware/CPU/Alu.cpp CPU src/Hardware/CPU/Alu.h)
add_library(Idu src/Hardware/CPU/Idu.cpp CPU src/Hardware/CPU/Idu.h)
add_library(Registers src/Hardware/CPU/RegisterFile.cpp src/Hardware/CPU/RegisterFile.h)
target_link_libraries(Cpu Alu Idu Registers)
#add_library(Timer src/Timer/Timer.h src/Timer/Timer.cpp)
target_link_libraries(Application SDL2::SDL2)
#target_link_libraries(DisplayHandler SDL2::SDL2)
#target_link_libraries(${PROJECT_NAME} Application MemoryManager DisplayHandler Timer)
target_link_libraries(${PROJECT_NAME} Application MemoryManager)

include_directories(${PROJECT_NAME} ${SDL2_LIBRARIES})