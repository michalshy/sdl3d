#include "shader.h"
#include "SDL3/SDL_gpu.h"
#include "SDL3/SDL_stdinc.h"

namespace shader
{
    SDL_GPUShader* LoadShader(
        SDL_GPUDevice* device,
        const char* shader_filename,
        Uint32 samplers,
        Uint32 uniform_buffers,
        Uint32 storage_buffers,
        Uint32 storage_textures
    )
    {
        // load the vertex shader code
        size_t shader_code_size;
        void* shader_code = SDL_LoadFile(shader_filename, &shader_code_size);

        SDL_GPUShaderStage stage;
        if(SDL_strstr(shader_filename, "vert"))
            stage = SDL_GPU_SHADERSTAGE_VERTEX;
        if(SDL_strstr(shader_filename, "frag"))
            stage = SDL_GPU_SHADERSTAGE_FRAGMENT;

        SDL_GPUShaderCreateInfo shader_info{};
        shader_info.code = (Uint8*)shader_code;
        shader_info.code_size = shader_code_size;
        shader_info.entrypoint = "main";
        shader_info.format = SDL_GPU_SHADERFORMAT_SPIRV;
        shader_info.stage = stage;
        shader_info.num_samplers = samplers;
        shader_info.num_storage_buffers = storage_buffers;
        shader_info.num_storage_textures = storage_textures;
        shader_info.num_uniform_buffers = uniform_buffers;

        SDL_GPUShader* shader = SDL_CreateGPUShader(device, &shader_info);
        if (shader == nullptr)
        {
            SDL_Log("Failed to create shader!");
            SDL_free(shader_code);
            return nullptr;
        }

        SDL_free(shader_code);
        return shader;
    }
}