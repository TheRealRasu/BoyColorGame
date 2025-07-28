#include "../Application/ApplicationDefines.h"
#include <cstdint>
#define SDL_MAIN_HANDLED
#pragma once

#include "SDL.h"

#include <array>
#include <memory>

class DisplayManager
{
public:
    DisplayManager();
    ~DisplayManager();

    void start();
    void stop();

    void renderImage();

    void reset();

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
    std::unique_ptr<SDL_Window, SdlWindowDtor> mWindow;

    std::array<uint8_t, gDisplayWidth * gDisplayHeight> mDisplayData;
};