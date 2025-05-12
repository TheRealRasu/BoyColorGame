#include "Application/Application.h"

#include <cstdlib>

bool gTerminate = false;

int main(char argc, char** argv)
{
    if (argc != 2) return EXIT_FAILURE;

    Application application;
    
    while (!gTerminate)
    {
        application.cpuLoop();
    }

    if (false) // TODO game loaded
    {
        const std::string gamePath = "foo";
        std::thread gameThread = std::thread(&Application::loadRom, application, gamePath);
    }

    return EXIT_SUCCESS;
}