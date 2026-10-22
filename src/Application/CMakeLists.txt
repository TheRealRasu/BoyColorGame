set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)
cmake_policy(SET CMP0079 NEW)

find_package(SDL2 REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SDL2_LIBRARIES})

add_library(Application
    Application.cpp
    Application.h
    ApplicationDefines.h
)

target_link_libraries(Application
    CpuCore
    Memory
    SDL2::SDL2
)