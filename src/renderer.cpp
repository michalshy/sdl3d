#include "renderer.h"

#include "shader.h"
#include "image.h"


struct Vertex {
    float x, y, z;
    float u, v;     // texture coords
};

static Vertex vertices[] = {
    //  x      y     z    u    v
    { -0.5f,  0.5f, 0.0f, 0.0f, 1.0f }, // top-left
    { -0.5f, -0.5f, 0.0f, 0.0f, 0.0f }, // bottom-left
    {  0.5f, -0.5f, 0.0f, 1.0f, 0.0f }, // bottom-right
    {  0.5f,  0.5f, 0.0f, 1.0f, 1.0f }  // top-right
};

static uint16_t indices[] = {
    0, 1, 2,  // first triangle
    0, 2, 3   // second triangle
};

Renderer::RenderData* Renderer::s_Data = nullptr;

bool Renderer::Init()
{
    s_Data = new RenderData();

    // create a window
    s_Data->window = SDL_CreateWindow("Skeletal Animations", 1280, 720, SDL_WINDOW_RESIZABLE);

    // create the device
    s_Data->device = SDL_CreateGPUDevice(SDL_GPU_SHADERFORMAT_SPIRV, true, NULL);
    SDL_ClaimWindowForGPUDevice(s_Data->device, s_Data->window);

    SDL_GPUShader* vertexShader = shader::LoadShader(s_Data->device, "res/shaders/compiled/texposvert.spv", 0, 0, 0, 0);
    SDL_GPUShader* fragmentShader = shader::LoadShader(s_Data->device, "res/shaders/compiled/texposfrag.spv", 1, 0, 0, 0);

    // create the graphics pipeline
    SDL_GPUGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.vertex_shader = vertexShader;
    pipelineInfo.fragment_shader = fragmentShader;
    pipelineInfo.primitive_type = SDL_GPU_PRIMITIVETYPE_TRIANGLELIST;

    // describe the vertex buffers
    SDL_GPUVertexBufferDescription vertexBufferDesctiptions[1];
    vertexBufferDesctiptions[0].slot = 0;
    vertexBufferDesctiptions[0].input_rate = SDL_GPU_VERTEXINPUTRATE_VERTEX;
    vertexBufferDesctiptions[0].instance_step_rate = 0;
    vertexBufferDesctiptions[0].pitch = sizeof(Vertex);

    pipelineInfo.vertex_input_state.num_vertex_buffers = 1;
    pipelineInfo.vertex_input_state.vertex_buffer_descriptions = vertexBufferDesctiptions;

    // describe the vertex attribute
    SDL_GPUVertexAttribute vertexAttributes[2];

    // a_position
    vertexAttributes[0].buffer_slot = 0;
    vertexAttributes[0].location = 0;
    vertexAttributes[0].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT3;
    vertexAttributes[0].offset = 0;

    // a_texcoord
    vertexAttributes[1].buffer_slot = 0;
    vertexAttributes[1].location = 1;
    vertexAttributes[1].format = SDL_GPU_VERTEXELEMENTFORMAT_FLOAT2;
    vertexAttributes[1].offset = sizeof(float) * 3;

    // describe the color target
    SDL_GPUColorTargetDescription colorTargetDescriptions[1];
    colorTargetDescriptions[0] = {};
    colorTargetDescriptions[0].format = SDL_GetGPUSwapchainTextureFormat(s_Data->device, s_Data->window);

    pipelineInfo.target_info.num_color_targets = 1;
    pipelineInfo.target_info.color_target_descriptions = colorTargetDescriptions;


    pipelineInfo.vertex_input_state.num_vertex_attributes = 2;
    pipelineInfo.vertex_input_state.vertex_attributes = vertexAttributes;

    // create the pipeline
    s_Data->graphicsPipeline = SDL_CreateGPUGraphicsPipeline(s_Data->device, &pipelineInfo);

    // we don't need to store the shaders after creating the pipeline
    SDL_ReleaseGPUShader(s_Data->device, vertexShader);
    SDL_ReleaseGPUShader(s_Data->device, fragmentShader);

    // Load 2d tex
    int width, height, channels;
    //SDL_Surface *img_data = image::LoadImage("res/textures/tex.bmp", 4);
    unsigned char* img_data = image::LoadImage("res/textures/container.jpg", &width, &height, &channels, 4);
    //width = img_data->w;
    //height = img_data->h;
    SDL_GPUTextureCreateInfo tex_info{};
    tex_info.type = SDL_GPU_TEXTURETYPE_2D;
    tex_info.format = SDL_GPU_TEXTUREFORMAT_R8G8B8A8_UNORM;
    tex_info.width = width;
    tex_info.height = height;
    tex_info.layer_count_or_depth = 1;
    tex_info.num_levels = 1;
    tex_info.usage = SDL_GPU_TEXTUREUSAGE_SAMPLER;

    s_Data->texture = SDL_CreateGPUTexture(s_Data->device, &tex_info);
	SDL_SetGPUTextureName(
		s_Data->device,
		s_Data->texture,
		"container"
	);

    SDL_GPUSamplerCreateInfo sampler_info{};
    sampler_info.min_filter = SDL_GPU_FILTER_NEAREST;
    sampler_info.mag_filter = SDL_GPU_FILTER_NEAREST;
    sampler_info.mipmap_mode = SDL_GPU_SAMPLERMIPMAPMODE_NEAREST;
    sampler_info.address_mode_u = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sampler_info.address_mode_v = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    sampler_info.address_mode_w = SDL_GPU_SAMPLERADDRESSMODE_CLAMP_TO_EDGE;
    s_Data->sampler = SDL_CreateGPUSampler(s_Data->device, &sampler_info);

    //////////// VERTEXES //////////////////////////////////////////
    // create the vertex buffer
    SDL_GPUBufferCreateInfo bufferInfo{};
    bufferInfo.size = sizeof(vertices);
    bufferInfo.usage = SDL_GPU_BUFFERUSAGE_VERTEX;
    s_Data->vertexBuffer = SDL_CreateGPUBuffer(s_Data->device, &bufferInfo);

    //////////// INDICES  //////////////////////////////////////////
    SDL_GPUBufferCreateInfo indicesInfo{};
    indicesInfo.size = sizeof(indices);
    indicesInfo.usage = SDL_GPU_BUFFERUSAGE_INDEX;
    s_Data->indexBuffer = SDL_CreateGPUBuffer(s_Data->device, &indicesInfo);

    // create a transfer buffer to upload vertices + indices
    SDL_GPUTransferBufferCreateInfo transferInfo{};
    transferInfo.size = sizeof(vertices) + sizeof(indices);
    transferInfo.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    s_Data->transfer_buff = SDL_CreateGPUTransferBuffer(s_Data->device, &transferInfo);

    // fill the transfer buffer: first vertices, then indices
    uint8_t* mapped = (uint8_t*)SDL_MapGPUTransferBuffer(s_Data->device, s_Data->transfer_buff, false);
    SDL_memcpy(mapped, vertices, sizeof(vertices));
    SDL_memcpy(mapped + sizeof(vertices), indices, sizeof(indices));
    SDL_UnmapGPUTransferBuffer(s_Data->device, s_Data->transfer_buff);
    ////////////////////////////////////////////////////////////////

    SDL_GPUTransferBufferCreateInfo transfer_tex{};
    transfer_tex.usage = SDL_GPU_TRANSFERBUFFERUSAGE_UPLOAD;
    transfer_tex.size = height * width * 4;

    SDL_GPUTransferBuffer* textureTransferBuffer = SDL_CreateGPUTransferBuffer(s_Data->device, &transfer_tex);
    Uint8* tex_data = (Uint8*)SDL_MapGPUTransferBuffer(s_Data->device, textureTransferBuffer, false);
    //SDL_memcpy(tex_data, img_data->pixels, width * height * 4);
    SDL_memcpy(tex_data, img_data, width * height * 4);
    SDL_UnmapGPUTransferBuffer(s_Data->device, textureTransferBuffer);

    // start a copy pass
    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(s_Data->device);
    SDL_GPUCopyPass* copyPass = SDL_BeginGPUCopyPass(commandBuffer);

    /// VERTEX
    // where is the data
    SDL_GPUTransferBufferLocation location{};
    location.transfer_buffer = s_Data->transfer_buff;
    location.offset = 0;
    // where to upload the data
    SDL_GPUBufferRegion region{};
    region.buffer = s_Data->vertexBuffer;
    region.size = sizeof(vertices);
    region.offset = 0;
    // upload the data
    SDL_UploadToGPUBuffer(copyPass, &location, &region, false);
    /// INDEX
    SDL_GPUTransferBufferLocation location_indices{};
    location_indices.transfer_buffer = s_Data->transfer_buff;
    location_indices.offset = sizeof(vertices);
    SDL_GPUBufferRegion region_indices{};
    region_indices.buffer = s_Data->indexBuffer;
    region_indices.size = sizeof(indices);
    region_indices.offset = 0;
    SDL_UploadToGPUBuffer(copyPass, &location_indices, &region_indices, false);

    SDL_GPUTextureTransferInfo texture_transfer_info{};
    texture_transfer_info.transfer_buffer = textureTransferBuffer;
    texture_transfer_info.offset = 0;
    SDL_GPUTextureRegion tex_region{};
    tex_region.texture = s_Data->texture;
    tex_region.w = width;
    tex_region.h = height;
    tex_region.d = 1;
    SDL_UploadToGPUTexture(
		copyPass,
		&texture_transfer_info,
        &tex_region,
		false
	);

    // end the copy pass
    SDL_EndGPUCopyPass(copyPass);
    SDL_SubmitGPUCommandBuffer(commandBuffer);

    return true;
}

void Renderer::PreRender()
{

}

void Renderer::Render()
{
    // acquire the command buffer
    SDL_GPUCommandBuffer* commandBuffer = SDL_AcquireGPUCommandBuffer(s_Data->device);

    // get the swapchain texture
    SDL_GPUTexture* swapchainTexture;
    Uint32 width, height;
    SDL_WaitAndAcquireGPUSwapchainTexture(commandBuffer, s_Data->window, &swapchainTexture, &width, &height);

    // end the frame early if a swapchain texture is not available
    if (swapchainTexture == NULL)
    {
        // you must always submit the command buffer
        SDL_SubmitGPUCommandBuffer(commandBuffer);
        return;
    }

    // create the color target
    SDL_GPUColorTargetInfo colorTargetInfo{};
    colorTargetInfo.clear_color = {240/255.0f, 240/255.0f, 240/255.0f, 255/255.0f};
    colorTargetInfo.load_op = SDL_GPU_LOADOP_CLEAR;
    colorTargetInfo.store_op = SDL_GPU_STOREOP_STORE;
    colorTargetInfo.texture = swapchainTexture;

    // begin a render pass
    SDL_GPURenderPass* renderPass = SDL_BeginGPURenderPass(commandBuffer, &colorTargetInfo, 1, NULL);

    // bind the pipeline
    SDL_BindGPUGraphicsPipeline(renderPass, s_Data->graphicsPipeline);

    SDL_GPUTextureSamplerBinding binding{};
    binding.texture = s_Data->texture;
    binding.sampler = s_Data->sampler;
    
    SDL_BindGPUFragmentSamplers(renderPass, 0, &binding, 1);
    // bind the buffers
    SDL_GPUBufferBinding vertex_bindings[1];
    vertex_bindings[0].buffer = s_Data->vertexBuffer; // index 0 is slot 0 in this example
    vertex_bindings[0].offset = 0; // start from the first byte
    SDL_GPUBufferBinding index_bindings[1];
    index_bindings[0].buffer = s_Data->indexBuffer;
    index_bindings[0].offset = 0;
    
    SDL_BindGPUVertexBuffers(renderPass, 0, vertex_bindings, 1); // bind one buffer starting from slot 0
    SDL_BindGPUIndexBuffer(renderPass, index_bindings, SDL_GPU_INDEXELEMENTSIZE_16BIT);
    SDL_DrawGPUIndexedPrimitives(renderPass, 6, 1, 0, 0, 0);

    SDL_EndGPURenderPass(renderPass);
    SDL_SubmitGPUCommandBuffer(commandBuffer);
}

void Renderer::PostRender()
{

}

void Renderer::Shutdown()
{
    // release the pipeline
    SDL_ReleaseGPUGraphicsPipeline(s_Data->device, s_Data->graphicsPipeline);

    SDL_ReleaseGPUTransferBuffer(s_Data->device, s_Data->transfer_buff);
    SDL_ReleaseGPUBuffer(s_Data->device, s_Data->vertexBuffer);
    
    // destroy the GPU device
    SDL_DestroyGPUDevice(s_Data->device);

    // destroy the window
    SDL_DestroyWindow(s_Data->window);

    delete s_Data;
}


