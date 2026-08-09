//
// Created by alan on 08/08/2026.
//

#include "Backend/DX12/DX12CommandQueue.h"
#include "Backend/DX12/DX12Device.h"

DX12CommandQueue::DX12CommandQueue(DX12Device* device) {
    m_Device = device;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    HRESULT hr = m_Device->GetDX12Device()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_Queue));
    m_CommandAllocatorPool.Init(m_Device->GetDX12Device());

    m_Fence = new DX12Fence(m_Device);
    m_Device->GetDX12Device()->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_CommandList));

    AcquireCommandAllocator();
}

DX12CommandQueue::~DX12CommandQueue() {
    m_Fence->Wait(m_FenceValue);
    if (m_Fence) {
        delete m_Fence;
    }
    if (m_CommandList) {
        m_CommandList->Release();
    }
    m_CommandAllocatorPool.Shutdown();
    if (m_Queue) {
        m_Queue->Release();
    }
}

void DX12CommandQueue::Wait(Fence* fence, uint64_t value) {
    DX12Fence* dxFence = static_cast<DX12Fence*>(fence);
    m_WaitFences.emplace_back(dxFence->GetDX12Fence(), value);
}

void DX12CommandQueue::Signal(Fence* fence, uint64_t value) {
    DX12Fence* dxFence = static_cast<DX12Fence*>(fence);
    m_SignalFences.emplace_back(dxFence->GetDX12Fence(), value);
}

void DX12CommandQueue::Flush() {
    m_CommandList->Close();
    m_FenceValue++;
    Signal(m_Fence, m_FenceValue);

    for (auto pair : m_WaitFences) {
        m_Queue->Wait(pair.first, pair.second);
    }

    ID3D12CommandList* commandLists = { m_CommandList };
    m_Queue->ExecuteCommandLists(1, &commandLists);

    for (auto pair : m_SignalFences) {
        m_Queue->Signal(pair.first, pair.second);
    }

    m_WaitFences.clear();
    m_SignalFences.clear();

    m_CommandAllocatorPool.ReleaseCommandAllocator(m_CommandAllocator, m_FenceValue);
    m_CommandAllocatorPool.Poll(m_FenceValue);
    AcquireCommandAllocator();
}

void DX12CommandQueue::AcquireCommandAllocator() {
    m_CommandAllocator = m_CommandAllocatorPool.AcquireCommandAllocator();
    m_CommandList->Reset(m_CommandAllocator, nullptr);
}