#define SDL_MAIN_USE_CALLBACKS 1
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3/SDL_gpu.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static SDL_Window* window = NULL;
static SDL_FPoint points[500];
static SDL_GPUBuffer* vbo = nullptr;
static bool debug_mode = false;
static SDL_GPUDevice* dev;
static SDL_GPUGraphicsPipeline* pipeline = nullptr;

static float vertices[] = {
    -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     0.0f,  0.5f, 0.0f
};

std::vector<Uint8> ReadBinaryFile(const char* path)
{
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        throw std::runtime_error("Cannot open file");
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<Uint8> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);

    return buffer;
}

SDL_AppResult SDL_AppInit(void** appstate, int argc, char* argv[])
{
    int i;

    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    window = SDL_CreateWindow("Hulia Engine", 1280, 720, NULL);
    if (!window) {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }

    dev = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, debug_mode, NULL);
    if (!dev) {
        SDL_Log("Failed to create GPU device");
        return SDL_APP_FAILURE;
    }
    // Claim window
    SDL_ClaimWindowForGPUDevice(dev, window);

    // Create shaders
    std::vector<Uint8> vert_shader = ReadBinaryFile("vert.spv");
    std::vector<Uint8> frag_shader = ReadBinaryFile("frag.spv");

    SDL_GPUShaderCreateInfo vert_info = {
        vert_shader.size(),
        &vert_shader[0],
        "main",
        SDL_GPU_SHADERFORMAT_SPIRV,
        SDL_GPU_SHADERSTAGE_VERTEX,
        0,
        0,
        0,
        0,
        0
    };

    SDL_GPUShaderCreateInfo frag_info = {
        frag_shader.size(),
        & frag_shader[0],
        "main",
        SDL_GPU_SHADERFORMAT_SPIRV,
        SDL_GPU_SHADERSTAGE_FRAGMENT,
        0,
        0,
        0,
        0,
        0
    };

    SDL_GPUShader* vert = SDL_CreateGPUShader(dev, &vert_info);
    SDL_GPUShader* frag = SDL_CreateGPUShader(dev, &frag_info);

    SDL_GPUBufferCreateInfo vertex_info{
        SDL_GPU_BUFFERUSAGE_VERTEX,
        sizeof(vertices),
        0
    };
    vbo = SDL_CreateGPUBuffer(dev, &vertex_info);

    SDL_GPUTransferBufferCreateInfo tbufInfo = {
        SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD,
        sizeof(vertices)
    };
    SDL_GPUTransferBuffer* transferBuffer = SDL_CreateGPUTransferBuffer(dev, &tbufInfo);

    SDL_GPUCommandBuffer* cmdBuf = SDL_AcquireGPUCommandBuffer(dev);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(cmdBuf);

    SDL_GPUTransferBufferLocation source = {
        transferBuffer,
        0
    };

    SDL_GPUBufferRegion destRegion = {
        vbo,
        0,
        sizeof(vertices)
    };

    SDL_UploadToGPUBuffer(copyPass, &source, &destRegion, false);

    SDL_GPUVertexAttribute vertexAttribs[] = {
    {
        0,          // matches shader input
        0,       // binding 0
        SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3,
        0
    }
    };

    SDL_GPUVertexBufferDescription vboDesc = {
        0,                          // matches the binding slot in your attribute
        sizeof(float) * 3,         // 3 floats per vertex = 12 bytes
        SDL_GPU_VERTEXINPUTRATE_VERTEX, // advance per vertex
        0             // must be 0 (reserved)
    };

    SDL_GPUVertexInputState vertexInputState = {
        &vboDesc,
        1,
        vertexAttribs,
        1
    };

    SDL_GPUGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.vertex_shader = vert;
    pipelineInfo.fragment_shader = frag;
    pipelineInfo.vertex_input_state = vertexInputState;
    pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

    // Use default rasterizer, multisample, depth-stencil, target info
    SDL_zero(pipelineInfo.rasterizer_state);
    SDL_zero(pipelineInfo.multisample_state);
    SDL_zero(pipelineInfo.depth_stencil_state);
    SDL_zero(pipelineInfo.target_info);

    pipeline = SDL_CreateGPUGraphicsPipeline(dev, &pipelineInfo);
    if (!pipeline) {
        SDL_Log("Failed to create graphics pipeline: %s", SDL_GetError());
        return SDL_APP_FAILURE;
    }
    return SDL_APP_CONTINUE;
}

SDL_AppResult SDL_AppEvent(void* appstate, SDL_Event* event)
{
    if (event->type == SDL_EVENT_QUIT) {
        return SDL_APP_SUCCESS; 
    }
    return SDL_APP_CONTINUE;
}

/* This function runs once per frame, and is the heart of the program. */
SDL_AppResult SDL_AppIterate(void* appstate)
{
    SDL_GPUCommandBuffer* cmdBuf = SDL_AcquireGPUCommandBuffer(dev);
    if (!cmdBuf) {
        SDL_Log("Failed to acquire GPU command buffer");
        return SDL_APP_CONTINUE;
    }
    SDL_GPUTexture* swapchainTex = NULL;
    Uint32 width = 0, height = 0;
    bool success = SDL_WaitAndAcquireGPUSwapchainTexture(cmdBuf, window, &swapchainTex, &width, &height);
    if (!success) {
        SDL_Log("SDL_WaitAndAcquireGPUSwapchainTexture failed: %s", SDL_GetError());
        SDL_SubmitGPUCommandBuffer(cmdBuf);
        return SDL_APP_CONTINUE;
    }

    if (swapchainTex == NULL) {
        SDL_SubmitGPUCommandBuffer(cmdBuf);
        return SDL_APP_CONTINUE;
    }

    SDL_GPUColorTargetInfo colorInfo;
    SDL_zero(colorInfo);
    colorInfo.texture = swapchainTex;
    colorInfo.clear_color = { 0.1f, 0.1f, 0.1f, 1.0f };
    colorInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorInfo.store_op = SDL_GPU_STOREOP_STORE;

    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(cmdBuf, &colorInfo, 1, NULL);

    SDL_GPUBufferBinding binding = {
        vbo,   // the GPU vertex buffer
        0      // start reading from the beginning
    };

    SDL_BindGPUGraphicsPipeline(renderPass, pipeline);     // your shader pipeline
    SDL_BindGPUVertexBuffers(renderPass, 0, &binding, 1);
    SDL_DrawGPUPrimitives(renderPass, 3, 1, 0, 0);
    SDL_EndGPURenderPass(renderPass);
    SDL_SubmitGPUCommandBuffer(cmdBuf);

    return SDL_APP_CONTINUE;
}

void SDL_AppQuit(void* appstate, SDL_AppResult result)
{
    // destroy the GPU device
    SDL_DestroyGPUDevice(dev);

    // destroy the window
    SDL_DestroyWindow(window);
}
