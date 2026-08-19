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
        .Width = 512,
        .Height = 512,
        .MipLevels = 1,
        .ArrayLayers = 1,
        .Samples = 1,
        .Format = TextureFormat::TEXTURE_FORMAT_B8G8R8A8_UNORM,
        .Type = TextureType::Texture2D
    };

    Texture* texture = device->CreateTexture(textureDesc);

    TextureViewDesc textureViewDesc = {
        .Tex = texture,
        .Format = TextureFormat::TEXTURE_FORMAT_B8G8R8A8_UNORM
    };

    TextureView* textureView = device->CreateTextureView(textureViewDesc);

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

    GraphicsPipelineDesc pipelineDesc = {
        .VertexShader = vertexShader,
        .FragmentShader = fragmentShader
    };
    pipelineState = device->CreateGraphicsPipelineState(pipelineDesc);

    while(!swapchain->WindowShouldClose()) {
        swapchain->UpdateWindow();

        queue->Barrier({ { swapchain->GetCurrentBackBuffer(), ResourceLayout::RenderTarget } });

        queue->SetGraphicsPipelineState(pipelineState);
        queue->SetRenderTargets({ swapchain->GetCurrentBackBuffer()->GetRTV() });
        queue->SetViewport({ 0, 0, 800, 600, 0.0f, 1.0f });
        queue->SetScissor({ 0, 0, 800, 600 });
        queue->DrawInstansed(3);

        queue->Barrier({ { swapchain->GetCurrentBackBuffer(), ResourceLayout::Present } });

        device->EndFrame();
        swapchain->Present();
    }

    delete textureView;
    delete texture;
    delete pipelineState;
    delete swapchain;
    delete device;

    ShaderCompiler::Shutdown();

    return 0;
}