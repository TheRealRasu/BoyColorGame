#pragma once

#include "../Application/ApplicationDefines.h"

#include <array>
#include <cstdint>
#include <memory>

#define SDL_MAIN_HANDLED
//#pragma once

class DisplayManager
{
public:
    DisplayManager();
    ~DisplayManager();

    void start();
    void stop();

    void renderImage();

private:
    struct SdlWindowDtor
    {
        void operator() (SDL_Window* window) const { SDL_DestroyWindow(window); }
    };

    struct SdlRendererDtor
    {
        void operator() (SDL_Renderer* renderer) const { SDL_DestroyRenderer(renderer); }
    };

    std::unique_ptr<SDL_Renderer, SdlRendererDtor> mRenderer;
    std::unique_ptr<SDL_Window, SdlWindowDtor> mRenderer;

    std::array<uint8_t, gDisplayWidth * gDisplayHeight> mDisplayData;
};