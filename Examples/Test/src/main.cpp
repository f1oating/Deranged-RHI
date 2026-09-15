//
// Created by alan on 12/08/2026.
//

#include "ShaderCompiler.h"
#include <cstring>
#include "Application.h"

#ifdef WIN32
#include <windows.h>
extern "C" { __declspec(dllexport) extern const UINT D3D12SDKVersion = 619;}
extern "C" { __declspec(dllexport) extern const char* D3D12SDKPath = ".\\D3D12\\"; }
#endif

int main() {
    Application application;

    Device* device = application.GetDevice();
    CommandQueue* queue = application.GetQueue();
    Swapchain* swapchain = application.GetSwapchain();
    GraphicsPipelineState* pipelineState = nullptr;

    BufferDesc cbufferDesc = {
        .Size = 256,
        .BindFlags = BUFFER_BIND_UNIFORM,
        .Usage = BufferUsage::Dynamic
    };
    Buffer* cbuffer = device->CreateBuffer(cbufferDesc);

    float vertices[] = {
        0.0f, 0.5f,
        0.5f, -0.5f,
        -0.5f, -0.5f
    };

    BufferDesc bufferDesc = {
        .Size = sizeof(vertices),
        .BindFlags = BUFFER_BIND_VERTEX,
        .Stride = 8,
        .Usage = BufferUsage::Default
    };
    Buffer* buffer = device->CreateBuffer(bufferDesc);
    queue->CopyToBuffer(buffer, sizeof(vertices), vertices);
    queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_VERTEX_INPUT,
    { { buffer, ACCESS_TRANSFER_WRITE, ACCESS_VERTEX_READ } }, {});

    auto vertexSource = ShaderCompiler::CompileShader("vertex.slang");
    Shader vertexShader{
        .Data = vertexSource.data(),
        .Size = vertexSource.size()
    };
    auto fragmentSource = ShaderCompiler::CompileShader("fragment.slang");
    Shader fragmentShader{
        .Data = fragmentSource.data(),
        .Size = fragmentSource.size()
    };

    VertexInputDesc inputDesc = {
        { { "POSITION", ValueType::Float2 } }
    };

    BlendDesc blendDesc = {
        .ColorAttachments = { {} }
    };

    GraphicsPipelineDesc pipelineDesc = {
        .VertexShader = vertexShader,
        .FragmentShader = fragmentShader,
        .VertexInput = inputDesc,
        .Blend = blendDesc,
        .ColorFormats = { TextureFormat::B8G8R8A8_UNORM },
        .DepthStencilFormat = TextureFormat::Unknown,
        .PrimitiveTopology = Topology::TriangleList
    };
    pipelineState = device->CreateGraphicsPipelineState(pipelineDesc);

    while(!application.WindowShouldClose()) {
        application.BeginFrame();

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

        Mesh cube = application.GetCubeMesh();
        queue->SetVertexBuffer(cube.Vertex);
        queue->SetIndexBuffer(cube.Index);

        glm::mat4 viewProj = application.GetCamera()->GetViewMatrix() * application.GetCamera()->GetProjectionMatrix();
        void* ptr = cbuffer->Map();
        memcpy(ptr, &viewProj, sizeof(glm::mat4));

        //queue->SetConstantBuffer("ViewProj", cbuffer);

        queue->DrawIndexedInstanced(cube.NumIndices);

        queue->Barrier(PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT, PIPELINE_STAGE_NONE, {},
            { { currentBackBuffer, ImageLayout::Present, ACCESS_COLOR_ATTACHMENT_WRITE, ACCESS_NONE } });

        application.EndFrame();
    }

    delete cbuffer;
    delete buffer;
    delete pipelineState;

    return 0;
}