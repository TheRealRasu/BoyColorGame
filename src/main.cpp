#include "Application/Application.h"

#include <cstdlib>

bool gTerminate = false;

int main(char argc, char** argv)
{
    if (argc < 2) return EXIT_FAILURE;

    Application application;
    while (!gTerminate)
    {
        application.cpuLoop();
    }

    return EXIT_SUCCESS;
}