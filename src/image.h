#pragma once

#include "SDL3/SDL.h"

namespace image
{
    SDL_Surface* LoadImage(const char* path, int desired_channels);
}