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

class DX12Device;

class DX12CommandQueue : public CommandQueue {
public:
    DX12CommandQueue(DX12Device* device);
    ~DX12CommandQueue() override;

    void Wait(Fence* fence, uint64_t value) override;
    void Signal(Fence* fence, uint64_t value) override;

    void Flush() override;

    ID3D12CommandQueue* GetDX12CommandQueue() const { return m_Queue; }

private:
    void AcquireCommandAllocator();

private:
    DX12Device* m_Device = nullptr;
    ID3D12CommandQueue* m_Queue = nullptr;
    CommandAllocatorPool m_CommandAllocatorPool;
    DX12Fence* m_Fence = nullptr;
    uint64_t m_FenceValue = 0;
    std::vector<std::pair<ID3D12Fence*, uint64_t>> m_WaitFences;
    std::vector<std::pair<ID3D12Fence*, uint64_t>> m_SignalFences;
    ID3D12GraphicsCommandList* m_CommandList = nullptr;
    ID3D12CommandAllocator* m_CommandAllocator = nullptr;

};

#endif //DERANGED_RHI_DX12COMMANDQUEUE_H
