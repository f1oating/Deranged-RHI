//
// Created by alan on 18/09/2026.
//

#include "Renderer.h"
#include <cstring>
#include "ShaderCompiler.h"

Renderer::Renderer(Device* device, CommandQueue* queue, Swapchain* swapchain) {
    m_Device = device;
    m_Queue = queue;
    m_Swapchain = swapchain;

    CreateDepthStencil();
    CreatePipelineState();

    BufferDesc transformCBufferDesc = {
        .Size = 256,
        .BindFlags = BUFFER_BIND_UNIFORM,
        .Usage = BufferUsage::Dynamic
    };
    m_TransformCBuffer = device->CreateBuffer(transformCBufferDesc);
    CreateSampler();
}

Renderer::~Renderer() {
    delete m_Sampler;
    delete m_TransformCBuffer;
    delete m_PipelineState;
    delete m_DepthStencil;
}

void Renderer::Render() {
    Texture* backBuffer = m_Swapchain->GetCurrentBackBuffer();
    TextureDesc backBufferDesc = backBuffer->GetDesc();

    m_Queue->Barrier(PIPELINE_STAGE_NONE, PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT, {},
        { { backBuffer, ImageLayout::RenderTarget, ACCESS_NONE, ACCESS_COLOR_ATTACHMENT_WRITE } });

    m_Queue->SetRenderTargets({ backBuffer->GetRTV() });
    m_Queue->SetDepthStencil(m_DepthStencil->GetDSV());
    m_Queue->ClearDepthStencil(1.0f, 0);
    m_Queue->ClearRenderTargets(0.0f, 0.0f, 0.0f, 1.0f);

    m_Queue->SetGraphicsPipelineState(m_PipelineState);
    m_Queue->SetViewport({ 0, 0, (float)backBufferDesc.Width, (float)backBufferDesc.Height, 0.0f, 1.0f });
    m_Queue->SetScissor({ 0, 0, (int)backBufferDesc.Width, (int)backBufferDesc.Height });

    for (auto& mesh : m_Meshes) {
        m_Queue->SetVertexBuffer(mesh.Vertex);
        m_Queue->SetIndexBuffer(mesh.Index);

        glm::mat4 projView = m_Camera->GetProjectionMatrix() * m_Camera->GetViewMatrix();
        void* transformCBufferPtr = m_TransformCBuffer->Map();
        memcpy(transformCBufferPtr, &projView, sizeof(glm::mat4));

        m_Queue->SetConstantBuffer("Transform", m_TransformCBuffer);
        m_Queue->SetTexture("Albedo", mesh.Albedo->GetSRV());
        m_Queue->SetSampler("Sampler", m_Sampler);

        m_Queue->DrawIndexedInstanced(mesh.NumIndices);
    }
    m_Meshes.clear();

    m_Queue->Barrier(PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT, PIPELINE_STAGE_NONE, {},
        { { backBuffer, ImageLayout::Present, ACCESS_COLOR_ATTACHMENT_WRITE, ACCESS_NONE } });

    m_Swapchain->Present();
}

void Renderer::AddMesh(Mesh mesh) {
    m_Meshes.push_back(mesh);
}

void Renderer::ResizeAttachments() {
    m_Swapchain->Resize();
    delete m_DepthStencil;
    CreateDepthStencil();
}

void Renderer::CreateDepthStencil() {
    TextureDesc backbufferDesc = m_Swapchain->GetCurrentBackBuffer()->GetDesc();
    TextureDesc depthStencilDesc = {
        .Width = backbufferDesc.Width,
        .Height = backbufferDesc.Height,
        .Format = TextureFormat::D32_FLOAT,
        .BindFlags = TEXTURE_BIND_DEPTH_STENCIL
    };
    m_DepthStencil = m_Device->CreateTexture(depthStencilDesc);
    m_Queue->Barrier(PIPELINE_STAGE_NONE, PIPELINE_STAGE_ALL_COMMANDS, {},
    { { m_DepthStencil, ImageLayout::DepthStencil, ACCESS_NONE, ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE } });
}

void Renderer::CreatePipelineState() {
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
        { { "POSITION", ValueType::Float3 }, { "TEXCOORD", ValueType::Float2 } }
    };

    BlendDesc blendDesc = {
        .ColorAttachments = { {} }
    };

    DepthStencilDesc depthStencilStateDesc = {
        .DepthEnable = true,
        .DepthWriteEnable = true,
        .DepthCompare = CompareOp::Less
    };

    GraphicsPipelineDesc pipelineDesc = {
        .VertexShader = vertexShader,
        .FragmentShader = fragmentShader,
        .VertexInput = inputDesc,
        .DepthStencil = depthStencilStateDesc,
        .Blend = blendDesc,
        .ColorFormats = { m_Swapchain->GetCurrentBackBuffer()->GetDesc().Format },
        .DepthStencilFormat = m_DepthStencil->GetDesc().Format,
        .PrimitiveTopology = Topology::TriangleList
    };
    m_PipelineState = m_Device->CreateGraphicsPipelineState(pipelineDesc);
}

void Renderer::CreateSampler() {
    SamplerDesc samplerDesc = {};
    m_Sampler = m_Device->CreateSampler(samplerDesc);
}