find_package(SDL2 REQUIRED)

include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${SDL2_LIBRARIES})

add_library(Display
    DisplayManager.cpp
    DisplayManager.h
)

target_link_libraries(Display SDL2::SDL2)