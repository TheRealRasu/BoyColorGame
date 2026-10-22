cmake_minimum_required(VERSION 3.7)
cmake_policy(SET CMP0079 NEW)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(SDL2 REQUIRED)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME} PRIVATE
    main.cpp
)

add_subdirectory(Application)
add_subdirectory(Display)
add_subdirectory(Hardware)

include_directories(${PROJECT_NAME}
    ${SDL2_LIBRARIES}
)

target_link_libraries(${PROJECT_NAME}
    Application
    Display
)
