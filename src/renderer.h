#pragma once

#include <SDL3/SDL_gpu.h>

class SDL_Window;
class SDL_GPUDevice;
class SDL_GPUTexture;
class SDL_GPUSampler;
class SDL_GPUBuffer;
class SDL_GPUTransferBuffer;
class SDL_GPUGraphicsPipeline;
class SDL_GPUShader;

class Renderer
{
public:
    static bool Init();
    static void Render();
    static void PreRender();
    static void PostRender();
    static void Shutdown();
private:
    struct RenderData
    {
        SDL_Window* window = nullptr;
        bool debug_mode = true;
        SDL_GPUDevice* device = nullptr;
        SDL_GPUTexture* texture = nullptr;
        SDL_GPUSampler* sampler = nullptr;
        SDL_GPUBuffer* vertexBuffer = nullptr;
        SDL_GPUBuffer* indexBuffer  = nullptr;
        SDL_GPUTransferBuffer* transfer_buff = nullptr;
        SDL_GPUGraphicsPipeline* graphicsPipeline = nullptr;
    };
    static RenderData* s_Data;
};