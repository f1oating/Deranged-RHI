//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_DX12COMMANDQUEUE_H
#define DERANGED_RHI_DX12COMMANDQUEUE_H

#include "CommandQueue.h"
#include <d3d12.h>
#include "Backend/DX12/Internal/CommandAllocatorPool.h"
#include "Backend/DX12/DX12Fence.h"
#include <vector>
#include "ReleaseManager.h"
#include "Backend/DX12/DX12Pipeline.h"
#include "Internal/DescriptorHeap.h"

namespace dx {

class DX12Device;

class DX12CommandQueue : public CommandQueue {
public:
    DX12CommandQueue(DX12Device* device);
    ~DX12CommandQueue() override;

    void Wait(Fence* fence, uint64_t value) override;
    void Signal(Fence* fence, uint64_t value) override;

    void SetGraphicsPipelineState(GraphicsPipelineState* graphicsPipelineState) override;

    void SetViewport(Viewport viewport) override;
    void SetScissor(Scissor scissor) override;

    void SetRenderTargets(std::vector<RenderTargetView*> rtvs) override;
    void SetDepthStencil(DepthStencilView *dsv) override;

    void ClearRenderTargets(float r, float g, float b, float a) override;
    void ClearDepthStencil(float depth, uint8_t stencil) override;

    void SetVertexBuffer(Buffer* buffer) override;
    void SetIndexBuffer(Buffer* buffer) override;

    void SetConstantBuffer(std::string name, Buffer* buffer) override;
    void SetTexture(std::string name, ShaderResourceView* textureView) override;
    void SetSampler(std::string name, Sampler* sampler) override;

    void DrawInstanced(uint32_t vertexCount, uint32_t instanceCount = 1,
        uint32_t startVertex = 0, uint32_t startInstance = 0) override;
    void DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount = 1,
        uint32_t startIndex = 0, uint32_t vertexOffset = 0, uint32_t startInstance = 0) override;

    void Barrier(uint32_t srcStage, uint32_t dstStage,
        std::vector<BufferBarrier> bufBarriers, std::vector<TextureBarrier> texBarriers) override;

    void CopyToBuffer(Buffer* dst, uint64_t size, void* data) override;
    void CopyToTexture(Texture* dst, uint64_t size, void* data) override;

    void Flush() override;

    void ReleaseResource(ReleaseResourceWrapper* resource);
    void EndFrame();

    ID3D12CommandQueue* GetDX12CommandQueue() const { return m_Queue; }

private:
    void MarkResourcesDirty();
    void BoundDirtyResources();
    void ClearRenderAttachmentsIfNeeded();

    void AcquireCommandAllocator();
    void SubmitCommandList();

private:
    DX12Device* m_Device = nullptr;

    ReleaseManager m_ReleaseManager;

    ID3D12CommandQueue* m_Queue = nullptr;

    ID3D12GraphicsCommandList7* m_CommandList = nullptr;
    ID3D12CommandAllocator* m_CommandAllocator = nullptr;
    uint64_t m_CommandAllocatorNumber = 0;
    std::unique_ptr<CommandAllocatorPool> m_CommandAllocatorPool = nullptr;

    DX12Fence* m_Fence = nullptr;
    std::vector<std::pair<ID3D12Fence*, uint64_t>> m_WaitFences;
    std::vector<std::pair<ID3D12Fence*, uint64_t>> m_SignalFences;

    std::unique_ptr<DescriptorsStateManager> m_DescriptorsStateManager = nullptr;

    DX12GraphicsPipelineState* m_GraphicsPipeline = nullptr;
    bool m_GraphicsPipelineBound = false;
    Viewport m_Viewport;
    bool m_ViewportBound = false;
    Scissor m_Scissor;
    bool m_ScissorBound = false;
    DX12Buffer* m_VertexBuffer = nullptr;
    bool m_VertexBufferBound = false;
    DX12Buffer* m_IndexBuffer = nullptr;
    bool m_IndexBufferBound = false;

    std::vector<DX12RenderTargetView*> m_RTVs;
    float m_RTVsClearValue[4];
    bool m_ShouldClearRTVs = false;
    DX12DepthStencilView* m_DSV = nullptr;
    float m_DSVDepthClearValue;
    uint8_t m_DSVStencilClearValue;
    bool m_ShouldClearDSV = false;
    bool m_RenderAttachmentsBound = false;

};

} // dx

#endif //DERANGED_RHI_DX12COMMANDQUEUE_H
