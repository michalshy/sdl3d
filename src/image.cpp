#include "image.h"

#define STB_IMAGE_IMPLEMENTATION
#include <SDL3/SDL.h>
#include "stb_image.h"



namespace image
{
    SDL_Surface* LoadImage(const char* path, int desired_channels)
    {
        SDL_Surface *result;
        SDL_PixelFormat format;

        result = SDL_LoadBMP(path);
        if (result == NULL)
        {
            SDL_Log("Failed to load BMP: %s", SDL_GetError());
            return NULL;
        }

        if (desired_channels == 4)
        {
            format = SDL_PIXELFORMAT_ABGR8888;
        }
        else
        {
            SDL_assert(!"Unexpected desiredChannels");
            SDL_DestroySurface(result);
            return NULL;
        }
        if (result->format != format)
        {
            SDL_Surface *next = SDL_ConvertSurface(result, format);
            SDL_DestroySurface(result);
            result = next;
        }

        return result;
    }

    unsigned char* LoadImage(const char* path, int* width, int* height, int* channels, int desired_channels)
    {
        return stbi_load(path, width, height, channels, desired_channels);
    }
}