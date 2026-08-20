//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_COMMANDQUEUE_H
#define DERANGED_RHI_COMMANDQUEUE_H

#include "Fence.h"
#include "Pipeline.h"
#include "Resource.h"
#include <vector>

struct Viewport {
    float TopLeftX;
    float TopLeftY;
    float Width;
    float Height;
    float MinDepth;
    float MaxDepth;
};

struct Scissor {
    int Left;
    int Top;
    int Right;
    int Bottom;
};

class CommandQueue {
public:
    virtual ~CommandQueue() = default;

    virtual void Wait(Fence* fence, uint64_t value) = 0;
    virtual void Signal(Fence* fence, uint64_t value) = 0;

    virtual void SetGraphicsPipelineState(GraphicsPipelineState* graphicsPipelineState) = 0;

    virtual void SetViewport(Viewport viewport) = 0;
    virtual void SetScissor(Scissor scissor) = 0;

    virtual void Barrier(std::vector<TextureBarrier> barriers) = 0;

    virtual void SetRenderTargets(std::vector<TextureView*> rtvs) = 0;
    virtual void ClearRenderTargets(float r, float g, float b, float a) = 0;

    virtual void DrawInstansed(uint32_t VertexCountPerInstance, uint32_t InstanceCount = 1,
        uint32_t StartVertexLocation = 0, uint32_t StartInstanceLocation = 0) = 0;

    virtual void Flush() = 0;

};

#endif //DERANGED_RHI_COMMANDQUEUE_H
