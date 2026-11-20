//
// Created by alan on 18/09/2026.
//

#include "Render/Renderer.h"
#include <cstring>
#include "ShaderCompiler.h"
#include "ThirdParty/stb_image.h"
#include "Render/RenderGraph.h"

Renderer::Renderer(Device* device, CommandQueue* queue, Swapchain* swapchain) {
    m_Device = device;
    m_Queue = queue;
    m_Swapchain = swapchain;

    CreateFallbackTexture();
    CreateCubeMesh();
    CreateTerrain();

    CreateConstantBuffers();
    CreateSampler();

    CreateDepthStencil();

    CreateMeshPipelineState();
    CreateTerrainPipelineState();
    CreateSkyboxPipelineState();
}

Renderer::~Renderer() {
    delete m_FallbackTexture;

    delete m_Cube.Vertex;
    delete m_Cube.Index;

    delete m_Terrain.Vertex;
    delete m_Terrain.Index;
    delete m_TerrainTexture;

    delete m_TransformCBuffer;
    delete m_LightCBuffer;
    delete m_Sampler;

    delete m_DepthStencil;

    delete m_MeshPipelineState;
    delete m_TerrainPipelineState;
    delete m_SkyboxPipelineState;
}

void Renderer::Render() {
    Texture* backBuffer = m_Swapchain->GetCurrentBackBuffer();
    TextureDesc backBufferDesc = backBuffer->GetDesc();

    m_Queue->Barrier(PIPELINE_STAGE_NONE, PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT, {},
        { { backBuffer, TextureLayout::RenderTarget, ACCESS_NONE, ACCESS_COLOR_ATTACHMENT_WRITE } });

    m_Queue->SetRenderTargets({ backBuffer->GetRTV() });
    m_Queue->SetDepthStencil(m_DepthStencil->GetDSV());
    m_Queue->ClearDepthStencil(1.0f, 0);
    m_Queue->ClearRenderTargets(0.0f, 0.0f, 0.0f, 1.0f);

    m_Queue->SetViewport({ 0, 0, (float)backBufferDesc.Width, (float)backBufferDesc.Height, 0.0f, 1.0f });
    m_Queue->SetScissor({ 0, 0, (int)backBufferDesc.Width, (int)backBufferDesc.Height });

    RenderGraph rg(m_Device);

    rg.AddPass("Mesh", [&](RenderGraph* rg) {
        m_Queue->SetGraphicsPipelineState(m_MeshPipelineState);

        for (auto* model : m_Models) {
            for (int i = 0; i < model->GetNumMeshes(); i++) {
                Mesh mesh = model->GetMesh(i);

                m_Queue->SetVertexBuffer(mesh.Vertex);
                m_Queue->SetIndexBuffer(mesh.Index);

                Transform transform{};
                transform.ModelViewProjection = m_Camera->GetProjectionMatrix() * m_Camera->GetViewMatrix() * model->GetWorldMatrix() * Y_FIX_MATRIX;
                transform.TransInvModelView = glm::transpose(glm::inverse(m_Camera->GetViewMatrix() * model->GetWorldMatrix())) * Y_FIX_MATRIX;
                transform.Model = model->GetWorldMatrix();

                void* transformCBufferPtr = m_TransformCBuffer->Map();
                memcpy(transformCBufferPtr, &transform, sizeof(Transform));

                Light light{};
                light.Directional = glm::mat3(transform.TransInvModelView) * glm::vec3(1.0f, 1.0f, 1.0f);
                void* lightCBufferPtr = m_LightCBuffer->Map();
                memcpy(lightCBufferPtr, &light, sizeof(Light));

                m_Queue->SetConstantBuffer("TransformBuffer", m_TransformCBuffer);
                m_Queue->SetConstantBuffer("LightBuffer", m_LightCBuffer);

                Material material = model->GetMaterial(mesh.MaterialIndex);
                m_Queue->SetTexture("Diffuse", material.Diffuse ? material.Diffuse->GetSRV() : m_FallbackTexture->GetSRV());
                m_Queue->SetTexture("Normal", material.Normal ? material.Normal->GetSRV() : m_FallbackTexture->GetSRV());
                m_Queue->SetSampler("Sampler", m_Sampler);

                m_Queue->DrawIndexedInstanced(mesh.NumIndices);
            }
        }
        m_Models.clear();
    }, {}, {});

    rg.AddPass("Terrain", [&](RenderGraph* rg) {
        m_Queue->SetGraphicsPipelineState(m_TerrainPipelineState);

        m_Queue->SetVertexBuffer(m_Terrain.Vertex);
        m_Queue->SetIndexBuffer(m_Terrain.Index);

        Transform transform{};
        transform.ModelViewProjection = m_Camera->GetProjectionMatrix() * m_Camera->GetViewMatrix() * Y_FIX_MATRIX;
        transform.TransInvModelView = glm::transpose(glm::inverse(m_Camera->GetViewMatrix())) * Y_FIX_MATRIX;
        transform.Model = glm::mat4(1);

        void* transformCBufferPtr = m_TransformCBuffer->Map();
        memcpy(transformCBufferPtr, &transform, sizeof(Transform));

        Light light{};
        light.Directional = glm::mat3(transform.TransInvModelView) * glm::vec3(1.0f, 1.0f, 1.0f);
        void* lightCBufferPtr = m_LightCBuffer->Map();
        memcpy(lightCBufferPtr, &light, sizeof(Light));

        m_Queue->SetConstantBuffer("TransformBuffer", m_TransformCBuffer);
        m_Queue->SetConstantBuffer("LightBuffer", m_LightCBuffer);

        m_Queue->SetTexture("Diffuse", m_TerrainTexture->GetSRV());
        m_Queue->SetSampler("Sampler", m_Sampler);

        m_Queue->DrawIndexedInstanced(m_Terrain.NumIndices);
    }, {}, {});

    rg.AddPass("Skybox", [&](RenderGraph* rg) {
        m_Queue->SetGraphicsPipelineState(m_SkyboxPipelineState);

        m_Queue->SetVertexBuffer(m_Cube.Vertex);
        m_Queue->SetIndexBuffer(m_Cube.Index);

        Transform transform{};
        transform.ModelViewProjection = m_Camera->GetProjectionMatrix() * glm::mat4(glm::mat3(m_Camera->GetViewMatrix())) * Y_FIX_MATRIX;
        transform.TransInvModelView = m_Camera->GetViewMatrix() * Y_FIX_MATRIX;
        transform.Model = glm::mat4(1);

        void* transformCBufferPtr = m_TransformCBuffer->Map();
        memcpy(transformCBufferPtr, &transform, sizeof(transform));

        m_Queue->SetConstantBuffer("TransformBuffer", m_TransformCBuffer);
        m_Queue->SetTexture("Skybox", m_Skybox);
        m_Queue->SetSampler("Sampler", m_Sampler);

        m_Queue->DrawIndexedInstanced(m_Cube.NumIndices);
    }, {}, {});

    rg.Compile();
    rg.Execute();

    m_Queue->Barrier(PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT, PIPELINE_STAGE_NONE, {},
        { { backBuffer, TextureLayout::Present, ACCESS_COLOR_ATTACHMENT_WRITE, ACCESS_NONE } });

    m_Swapchain->Present();
}

void Renderer::AddModel(Model* model) {
    m_Models.push_back(model);
}

void Renderer::ResizeAttachments() {
    m_Swapchain->Resize();
    delete m_DepthStencil;
    CreateDepthStencil();
}

void Renderer::CreateFallbackTexture() {
    TextureDesc texDesc = {
        .BindFlags = TEXTURE_BIND_SHADER_RESOURCE | TEXTURE_BIND_TRANSFER_DST
    };

    m_FallbackTexture = m_Device->CreateTexture(texDesc);

    struct Color {
        uint8_t r;
        uint8_t g;
        uint8_t b;
        uint8_t a;
    };
    Color color{};
    color.r = 255;
    color.a = 255;

    TextureSubresourceLayers layer = {
        .ArrayLayer = 0,
        .ArraysCount = 1
    };

    m_Queue->Barrier(PIPELINE_STAGE_NONE, PIPELINE_STAGE_TRANSFER, {}, {
        { m_FallbackTexture, TextureLayout::TransferDST, ACCESS_NONE, ACCESS_TRANSFER_WRITE }
    });

    m_Queue->CopyToTexture(m_FallbackTexture, sizeof(Color), &color, layer);

    m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_FRAGMENT_SHADER, {}, {
        { m_FallbackTexture, TextureLayout::ShaderResource, ACCESS_TRANSFER_WRITE, ACCESS_SHADER_READ }
    });
}

void Renderer::CreateCubeMesh() {
    glm::f32 x = 1.0f / 2.0f;
    glm::f32 y = 1.0f / 2.0f;
    glm::f32 z = 1.0f / 2.0f;

    glm::vec3 a0 = glm::vec3(+x, +y, +z);
    glm::vec3 a1 = glm::vec3(-x, +y, +z);
    glm::vec3 a2 = glm::vec3(-x, -y, +z);
    glm::vec3 a3 = glm::vec3(+x, -y, +z);
    glm::vec3 a4 = glm::vec3(+x, +y, -z);
    glm::vec3 a5 = glm::vec3(-x, +y, -z);
    glm::vec3 a6 = glm::vec3(-x, -y, -z);
    glm::vec3 a7 = glm::vec3(+x, -y, -z);

    glm::vec3 verts[] = {
        a1, a2, a3, a3, a0, a1,
        a2, a6, a7, a7, a3, a2,
        a6, a5, a4, a4, a7, a6,
        a5, a1, a0, a0, a4, a5,
        a0, a3, a7, a7, a4, a0,
        a5, a6, a2, a2, a1, a5
    };

    glm::vec2 texc[] = {
        glm::vec2(0,1), glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,0), glm::vec2(1,1), glm::vec2(0,1),
        glm::vec2(0,1), glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,0), glm::vec2(1,1), glm::vec2(0,1),
        glm::vec2(1,0), glm::vec2(1,1), glm::vec2(0,1), glm::vec2(0,1), glm::vec2(0,0), glm::vec2(1,0),
        glm::vec2(0,1), glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,0), glm::vec2(1,1), glm::vec2(0,1),
        glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,1), glm::vec2(1,1), glm::vec2(0,1), glm::vec2(0,0),
        glm::vec2(1,1), glm::vec2(0,1), glm::vec2(0,0), glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,1),
    };

    glm::vec3 norm[36];

    for (int i = 0; i < 36; i += 3)
    {
        glm::vec3 normal = glm::normalize(
            glm::cross(
                glm::vec3(verts[i + 1]) - glm::vec3(verts[i]),
                glm::vec3(verts[i + 2]) - glm::vec3(verts[i])));

        norm[i] = normal;
        norm[i + 1] = normal;
        norm[i + 2] = normal;
    }

    float vertices[36 * 8];
    for (int i = 0; i < 36; i++)
    {
        vertices[i * 8 + 0] = verts[i].x;
        vertices[i * 8 + 1] = verts[i].y;
        vertices[i * 8 + 2] = verts[i].z;

        vertices[i * 8 + 3] = texc[i].x;
        vertices[i * 8 + 4] = texc[i].y;

        vertices[i * 8 + 5] = norm[i].x;
        vertices[i * 8 + 6] = norm[i].y;
        vertices[i * 8 + 7] = norm[i].z;
    }


    BufferDesc vertexDesc = {
        .Size = sizeof(vertices),
        .BindFlags = BUFFER_BIND_VERTEX | BUFFER_BIND_TRANSFER_DST,
        .Stride = sizeof(VertexPositionTextureNormal),
        .Usage = BufferUsage::Default
    };

    uint32_t indices[] = {
        0, 1, 2, 3, 4, 5,
        6, 7, 8, 9, 10, 11,
        12, 13, 14, 15, 16, 17,
        18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29,
        30, 31, 32, 33, 34, 35,
    };

    BufferDesc indexDesc = {
        .Size = sizeof(indices),
        .BindFlags = BUFFER_BIND_INDEX | BUFFER_BIND_TRANSFER_DST,
        .Usage = BufferUsage::Default
    };

    m_Cube.Vertex = m_Device->CreateBuffer(vertexDesc);
    m_Cube.Index = m_Device->CreateBuffer(indexDesc);
    m_Cube.NumIndices = sizeof(indices) / sizeof(uint32_t);

    m_Queue->CopyToBuffer(m_Cube.Vertex, sizeof(vertices), vertices);
    m_Queue->CopyToBuffer(m_Cube.Index, sizeof(indices), indices);

    m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_VERTEX_INPUT,
    { { m_Cube.Vertex, ACCESS_TRANSFER_WRITE, ACCESS_VERTEX_READ } }, {});
    m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_INDEX_INPUT,
    { { m_Cube.Index, ACCESS_TRANSFER_WRITE, ACCESS_INDEX_READ } }, {});
}

void Renderer::CreateTerrain() {
    int width, height, nChannels;
    unsigned char* data = stbi_load("resources/terrain_heightmap.png", &width, &height, &nChannels, STBI_default);

    std::vector<VertexPositionTextureNormal> vertices;
    std::vector<uint32_t> indices;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            uint32_t texelIndex = (x + y * width) * nChannels;
            unsigned char texelData = data[texelIndex];

            VertexPositionTextureNormal vert{};
            vert.Position.x = -width / 2 + x;
            vert.Position.y = texelData * (64.0f / 256.0f) - 16.0f;
            vert.Position.z = -height / 2 + y;
            vert.UV.x = (float)x / (float)(width % 32);
            vert.UV.y = (float)y / (float)(height % 32);

            float hl = data[(std::max(0, x - 1) + y * width) * nChannels];
            float hr = data[(std::min(x + 1, width) + y * width) * nChannels];
            float hd = data[(x + std::min(y + 1, height) * width) * nChannels];
            float hu = data[(x + std::max(0, y - 1) * width) * nChannels];

            glm::vec3 normal{};
            normal.x = hl - hr;
            normal.y = 2.0f;
            normal.z = hd - hu;
            normal = glm::normalize(normal);
            vert.Normal = normal;

            vertices.push_back(vert);
        }
    }

    for (int x = 0; x < width - 1; x++) {
        for (int y = 0; y < height - 1; y++) {
            indices.push_back(x + y * width);
            indices.push_back(x + 1 + y * width);
            indices.push_back(x + (y + 1) * width);
            indices.push_back(x + 1 + y * width);
            indices.push_back(x + 1 + (y + 1) * width);
            indices.push_back(x + (y + 1) * width);
        }
    }

    stbi_image_free(data);

    BufferDesc vbDesc = {
        .Size = vertices.size() * sizeof(VertexPositionTextureNormal),
        .BindFlags = BUFFER_BIND_VERTEX | BUFFER_BIND_TRANSFER_DST,
        .Stride = sizeof(VertexPositionTextureNormal)
    };

    BufferDesc ibDesc = {
        .Size = indices.size() * sizeof(uint32_t),
        .BindFlags = BUFFER_BIND_INDEX | BUFFER_BIND_TRANSFER_DST,
    };

    data = stbi_load("resources/ground.jpg", &width, &height, &nChannels, 4);

    TextureDesc texDesc = {
        .Width = (uint32_t)width,
        .Height = (uint32_t)height
    };

    m_Terrain.Vertex = m_Device->CreateBuffer(vbDesc);
    m_Terrain.Index = m_Device->CreateBuffer(ibDesc);
    m_Terrain.NumIndices = indices.size();
    m_TerrainTexture = m_Device->CreateTexture(texDesc);

    m_Queue->Barrier(PIPELINE_STAGE_NONE, PIPELINE_STAGE_TRANSFER,
    {}, { { m_TerrainTexture, TextureLayout::TransferDST, ACCESS_NONE, ACCESS_TRANSFER_WRITE } });

    m_Queue->CopyToTexture(m_TerrainTexture, width * height * 4, data, { 0, 1 });
    m_Queue->CopyToBuffer(m_Terrain.Vertex, vertices.size() * sizeof(VertexPositionTextureNormal), vertices.data());
    m_Queue->CopyToBuffer(m_Terrain.Index, indices.size() * sizeof(uint32_t), indices.data());

    m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_FRAGMENT_SHADER,
    {}, { { m_TerrainTexture, TextureLayout::ShaderResource, ACCESS_TRANSFER_WRITE, ACCESS_SHADER_READ } });
    m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_VERTEX_INPUT,
    { { m_Terrain.Vertex, ACCESS_TRANSFER_WRITE, ACCESS_VERTEX_READ } }, {});
    m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_INDEX_INPUT,
    { { m_Terrain.Index, ACCESS_TRANSFER_WRITE, ACCESS_INDEX_READ } }, {});
}

void Renderer::CreateConstantBuffers() {
    BufferDesc transformCBufferDesc = {
        .Size = sizeof(Transform),
        .BindFlags = BUFFER_BIND_UNIFORM,
        .Usage = BufferUsage::Dynamic
    };
    m_TransformCBuffer = m_Device->CreateBuffer(transformCBufferDesc);

    BufferDesc lightCBufferDesc = {
        .Size = sizeof(Light),
        .BindFlags = BUFFER_BIND_UNIFORM,
        .Usage = BufferUsage::Dynamic
    };
    m_LightCBuffer = m_Device->CreateBuffer(lightCBufferDesc);
}

void Renderer::CreateSampler() {
    SamplerDesc samplerDesc = {};
    m_Sampler = m_Device->CreateSampler(samplerDesc);
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
    { { m_DepthStencil, TextureLayout::DepthStencil, ACCESS_NONE, ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE } });
}

void Renderer::CreateMeshPipelineState() {
    auto vertexSource = ShaderCompiler::CompileShader("MeshV.slang");
    Shader vertexShader{
        .Data = vertexSource.data(),
        .Size = vertexSource.size()
    };
    auto fragmentSource = ShaderCompiler::CompileShader("MeshF.slang");
    Shader fragmentShader{
        .Data = fragmentSource.data(),
        .Size = fragmentSource.size()
    };

    VertexInputDesc inputDesc = {
        {
            { "POSITION", ValueType::Float3 },
            { "TEXCOORD", ValueType::Float2 },
            { "NORMAL", ValueType::Float3 },
            { "TANGENT", ValueType::Float3 },
            { "BITANGENT", ValueType::Float3 },
        }
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
    m_MeshPipelineState = m_Device->CreateGraphicsPipelineState(pipelineDesc);
}

void Renderer::CreateTerrainPipelineState() {
    auto vertexSource = ShaderCompiler::CompileShader("TerrainV.slang");
    Shader vertexShader{
        .Data = vertexSource.data(),
        .Size = vertexSource.size()
    };
    auto fragmentSource = ShaderCompiler::CompileShader("TerrainF.slang");
    Shader fragmentShader{
        .Data = fragmentSource.data(),
        .Size = fragmentSource.size()
    };

    VertexInputDesc inputDesc = {
        {
            { "POSITION", ValueType::Float3 },
            { "TEXCOORD", ValueType::Float2 },
            { "NORMAL", ValueType::Float3 }
        }
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
    m_TerrainPipelineState = m_Device->CreateGraphicsPipelineState(pipelineDesc);
}

void Renderer::CreateSkyboxPipelineState() {
    auto vertexSource = ShaderCompiler::CompileShader("SkyboxV.slang");
    Shader vertexShader{
        .Data = vertexSource.data(),
        .Size = vertexSource.size()
    };
    auto fragmentSource = ShaderCompiler::CompileShader("SkyboxF.slang");
    Shader fragmentShader{
        .Data = fragmentSource.data(),
        .Size = fragmentSource.size()
    };

    VertexInputDesc inputDesc = {
        {
            { "POSITION", ValueType::Float3 },
            { "TEXCOORD", ValueType::Float2 },
            { "NORMAL", ValueType::Float3 } }
    };

    BlendDesc blendDesc = {
        .ColorAttachments = { {} }
    };

    DepthStencilDesc depthStencilStateDesc = {
        .DepthEnable = true,
        .DepthWriteEnable = true,
        .DepthCompare = CompareOp::LessOrEqual
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
    m_SkyboxPipelineState = m_Device->CreateGraphicsPipelineState(pipelineDesc);
}