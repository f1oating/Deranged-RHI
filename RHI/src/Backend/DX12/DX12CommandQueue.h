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

    void DrawInstaned(uint32_t VertexCountPerInstance, uint32_t InstanceCount = 1,
        uint32_t StartVertexLocation = 0, uint32_t StartInstanceLocation = 0) override;

    void Flush() override;

    void ReleaseResource(ReleaseResourceWrapper* resource);
    void EndFrame();

    ID3D12CommandQueue* GetDX12CommandQueue() const { return m_Queue; }

private:
    void AcquireCommandAllocator();
    void SubmitCommandList();

private:
    DX12Device* m_Device = nullptr;
    ID3D12CommandQueue* m_Queue = nullptr;
    dx::CommandAllocatorPool m_CommandAllocatorPool;
    DX12Fence* m_Fence = nullptr;
    std::vector<std::pair<ID3D12Fence*, uint64_t>> m_WaitFences;
    std::vector<std::pair<ID3D12Fence*, uint64_t>> m_SignalFences;
    ID3D12GraphicsCommandList* m_CommandList = nullptr;
    ID3D12CommandAllocator* m_CommandAllocator = nullptr;
    uint64_t m_CommandAllocatorNumber = 0;
    ReleaseManager m_ReleaseManager;

};

} // dx

#endif //DERANGED_RHI_DX12COMMANDQUEUE_H
