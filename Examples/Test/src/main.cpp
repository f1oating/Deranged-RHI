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

        device->EndFrame();
        swapchain->Present();
    }

    delete pipelineState;
    delete swapchain;
    delete device;

    ShaderCompiler::Shutdown();

    return 0;
}