#pragma once

class SDL_Surface;

namespace image
{
    SDL_Surface* LoadImage(const char* path, int desired_channels);
    float* LoadHDRImage(const char* imageFilename, int* pWidth, int* pHeight, int* pChannels, int desiredChannels);
}