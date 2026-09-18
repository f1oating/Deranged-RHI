//
// Created by alan on 18/09/2026.
//

#ifndef DERANGED_RHI_RENDERER_H
#define DERANGED_RHI_RENDERER_H

#include "Device.h"
#include <vector>
#include "Camera.h"

struct Mesh {
    Buffer* Vertex;
    Buffer* Index;
    uint32_t NumIndices;
};

class Renderer {
public:
    Renderer(Device* device, CommandQueue* queue, Swapchain* swapchain);
    ~Renderer();

    void BeginFrame();
    void EndFrame();

    void Render(Mesh mesh);

    void ResizeAttachments();

    void SetCamera(Camera* camera) { m_Camera = camera; }

private:
    void CreateDepthStencil();
    void CreatePipelineState();

private:
    Device* m_Device = nullptr;
    CommandQueue* m_Queue = nullptr;
    Swapchain* m_Swapchain = nullptr;

    Camera* m_Camera = nullptr;
    std::vector<Mesh> m_Meshes;

    Texture* m_DepthStencil = nullptr;
    GraphicsPipelineState* m_PipelineState = nullptr;
    Buffer* m_TransformCBuffer = nullptr;

};

#endif //DERANGED_RHI_RENDERER_H
