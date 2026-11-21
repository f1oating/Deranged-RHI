//
// Created by alan on 18/09/2026.
//

#ifndef DERANGED_RHI_RENDERER_H
#define DERANGED_RHI_RENDERER_H

#include "Device.h"
#include <vector>
#include "Render/Camera.h"
#include "Render/Scene.h"

#ifdef WIN32
#define SYSTEM_FIX_MATRIX glm::scale(glm::mat4(1.0f), { 1.0f, 1.0f, 1.0f })
#else
#define SYSTEM_FIX_MATRIX glm::scale(glm::mat4(1.0f), { 1.0f, -1.0f, 1.0f })
#endif

class Renderer {
public:
    Renderer(Device* device, CommandQueue* queue, Swapchain* swapchain);
    ~Renderer();

    void AddModel(Model* model);

    void Render(double time);

    void ResizeAttachments();

    void SetCamera(Camera* camera) { m_Camera = camera; }
    void SetSkybox(TextureView* skybox) { m_Skybox = skybox; }

private:
    void CreateFallbackTexture();
    void CreateCubeMesh();
    void CreateTerrain();

    void CreateConstantBuffers();
    void CreateSampler();

    void CreateDepthStencil();

    void CreateMeshPipelineState();
    void CreateTerrainPipelineState();
    void CreateSkyboxPipelineState();
    void CreateDebugLinesPipelineState();

private:
    Device* m_Device = nullptr;
    CommandQueue* m_Queue = nullptr;
    Swapchain* m_Swapchain = nullptr;

    Camera* m_Camera = nullptr;

    std::vector<Model*> m_Models;
    TextureView* m_Skybox = nullptr;

    Buffer* m_TransformCBuffer = nullptr;
    Buffer* m_LightCBuffer = nullptr;
    Sampler* m_Sampler = nullptr;

    Texture* m_DepthStencil = nullptr;

    GraphicsPipelineState* m_MeshPipelineState = nullptr;
    GraphicsPipelineState* m_TerrainPipelineState = nullptr;
    GraphicsPipelineState* m_SkyboxPipelineState = nullptr;

    GraphicsPipelineState* m_DebugLinesPipelineState = nullptr;

    Texture* m_FallbackTexture = nullptr;
    Mesh m_Cube;
    Texture* m_TerrainTexture = nullptr;
    Mesh m_Terrain{};

};

#endif //DERANGED_RHI_RENDERER_H
