#include "Application/Application.h"

#include <cstdlib>
#include <thread>

bool gTerminate = false;

int main(int argc, char** argv)
{
    if (argc != 2) return EXIT_FAILURE;

    Application application;
    
    while (!gTerminate)
    {
        application.loop();
    }

    if (false) // TODO game loaded
    {
        const std::string gamePath = "foo";
        std::thread gameThread = std::thread(&Application::loadRom, &application, gamePath);
    }

    return EXIT_SUCCESS;
}