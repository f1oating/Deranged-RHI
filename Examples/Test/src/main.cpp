//
// Created by alan on 12/08/2026.
//

#include "Device.h"
#include "ShaderCompiler.h"

#ifdef WIN32
#include <windows.h>
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 619;}
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }
#endif

int main() {
    ShaderCompiler::Init();

    Device* device = Device::Create();
    CommandQueue* queue = device->GetCommandQueue();
    Swapchain* swapchain = device->CreateSwapchain();
    GraphicsPipelineState* pipelineState;

    TextureDesc textureDesc = {
        .Width = 32,
        .Height = 32,
        .MipLevels = 1,
        .ArrayLayers = 1,
        .Samples = 1,
        .Format = TextureFormat::B8G8R8A8_UNORM,
        .Type = TextureType::Texture2D,
        .BindFlags = TEXTURE_BIND_RENDER_TARGET | TEXTURE_BIND_SHADER_RESOURCE
    };

    Texture* texture = device->CreateTexture(textureDesc);

    TextureViewDesc textureViewDesc = {
        .Tex = texture,
        .Format = TextureFormat::B8G8R8A8_UNORM
    };

    TextureView* textureView = device->CreateTextureView(textureViewDesc);

    float vertices[] = {
        0.0f, 0.5f,
        0.5f, -0.5f,
        -0.5f, -0.5f
    };

    BufferDesc bufferDesc = {
        .Size = sizeof(vertices),
        .BindFlags = BUFFER_BIND_VERTEX,
        .Usage = BufferUsage::Default
    };
    Buffer* buffer = device->CreateBuffer(bufferDesc);
    queue->CopyToBuffer(buffer, sizeof(vertices), vertices);
    queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_VERTEX_INPUT,
    { { buffer, ACCESS_TRANSFER_WRITE, ACCESS_SHADER_READ } }, {});

    auto vertexSource = ShaderCompiler::CompileShader("shaders/vertex.slang");
    Shader vertexShader{
        .Data = vertexSource.data(),
        .Size = vertexSource.size()
    };
    auto fragmentSource = ShaderCompiler::CompileShader("shaders/fragment.slang");
    Shader fragmentShader{
        .Data = fragmentSource.data(),
        .Size = fragmentSource.size()
    };

    VertexInputDesc inputDesc = {
        { { "POSITION", ValueType::Float2 } }
    };

    GraphicsPipelineDesc pipelineDesc = {
        .VertexShader = vertexShader,
        .FragmentShader = fragmentShader,
        .VertexInput = inputDesc
    };
    pipelineState = device->CreateGraphicsPipelineState(pipelineDesc);

    while(!swapchain->WindowShouldClose()) {
        swapchain->UpdateWindow();

        Texture* currentBackBuffer = swapchain->GetCurrentBackBuffer();
        TextureDesc backBufferDesc = currentBackBuffer->GetDesc();

        queue->Barrier(PIPELINE_STAGE_NONE, PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT, {},
            { { currentBackBuffer, ImageLayout::RenderTarget, ACCESS_NONE, ACCESS_COLOR_ATTACHMENT_READ } });

        queue->SetGraphicsPipelineState(pipelineState);
        queue->SetRenderTargets({ swapchain->GetCurrentBackBuffer()->GetRTV() });
        queue->ClearRenderTargets(0.1f, 0.2f, 0.3f, 1.0f);

        queue->SetViewport({ 0, 0, (float)backBufferDesc.Width,
            (float)backBufferDesc.Height, 0.0f, 1.0f });
        queue->SetScissor({ 0, 0, (int)backBufferDesc.Width, (int)backBufferDesc.Height });

        queue->SetVertexBuffer(buffer, 8);
        queue->DrawInstansed(3);

        queue->Barrier(PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT, PIPELINE_STAGE_NONE, {},
            { { currentBackBuffer, ImageLayout::Present, ACCESS_COLOR_ATTACHMENT_WRITE, ACCESS_NONE } });

        buffer->Map();

        device->EndFrame();
        swapchain->Present();
    }

    delete buffer;
    delete textureView;
    delete texture;
    delete pipelineState;
    delete swapchain;
    delete device;

    ShaderCompiler::Shutdown();

    return 0;
}