//
// Created by alan on 18/09/2026.
//

#ifndef DERANGED_RHI_RENDERER_H
#define DERANGED_RHI_RENDERER_H

#include "Device.h"
#include <vector>
#include "Render/Camera.h"
#include "Render/Model.h"

class Renderer {
public:
    Renderer(Device* device, CommandQueue* queue, Swapchain* swapchain);
    ~Renderer();

    void AddModel(Model* model);

    void Render();

    void ResizeAttachments();

    void SetCamera(Camera* camera) { m_Camera = camera; }
    void SetSkybox(TextureView* skybox) { m_Skybox = skybox; }

private:
    void CreateFallbackTexture();
    void CreateCubeMesh();

    void CreateDepthStencil();
    void CreatePipelineState();
    void CreateSkyboxPipelineState();
    void CreateSampler();

private:
    Device* m_Device = nullptr;
    CommandQueue* m_Queue = nullptr;
    Swapchain* m_Swapchain = nullptr;

    Camera* m_Camera = nullptr;

    std::vector<Model*> m_Models;

    TextureView* m_Skybox = nullptr;

    Texture* m_FallbackTexture = nullptr;
    Mesh m_Cube;

    Texture* m_DepthStencil = nullptr;
    GraphicsPipelineState* m_PipelineState = nullptr;
    GraphicsPipelineState* m_SkyboxPipelineState = nullptr;
    Buffer* m_TransformCBuffer = nullptr;
    Sampler* m_Sampler = nullptr;

};

#endif //DERANGED_RHI_RENDERER_H
