#pragma once

class SDL_Surface;

namespace image
{
    SDL_Surface* LoadImage(const char* path, int desired_channels);
    unsigned char* LoadImage(const char* imageFilename, int* pWidth, int* pHeight, int* pChannels, int desiredChannels);
}