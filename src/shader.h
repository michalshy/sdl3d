#pragma once

#include <SDL3/SDL.h>
#include <SDL3/SDL_gpu.h>

namespace shader
{
    SDL_GPUShader* LoadShader(
        SDL_GPUDevice* device,
        const char* shader_filename,
        Uint32 samplers,
        Uint32 uniform_buffers,
        Uint32 storage_buffers,
        Uint32 storage_textures
    );
}