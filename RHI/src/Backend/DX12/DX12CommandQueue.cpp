//
// Created by alan on 08/08/2026.
//

#include "Backend/DX12/DX12CommandQueue.h"

#include <iso646.h>

#include "Backend/DX12/DX12Device.h"

namespace dx {

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
    SubmitCommandList();
    m_Fence->Wait(m_CommandAllocatorNumber);
    m_ReleaseManager.Clear();
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

void DX12CommandQueue::SetGraphicsPipelineState(GraphicsPipelineState* graphicsPipelineState) {
    DX12GraphicsPipelineState* dxGraphicsPipelineState = static_cast<DX12GraphicsPipelineState*>(graphicsPipelineState);

    m_CommandList->SetGraphicsRootSignature(dxGraphicsPipelineState->GetRootSignature());
    m_CommandList->SetPipelineState(dxGraphicsPipelineState->GetPipelineState());
}

void DX12CommandQueue::SetViewport(Viewport viewport) {
    D3D12_VIEWPORT dxViewport = {
        .TopLeftX = viewport.TopLeftX,
        .TopLeftY = viewport.TopLeftY,
        .Width = viewport.Width,
        .Height = viewport.Height,
        .MinDepth = viewport.MinDepth,
        .MaxDepth = viewport.MaxDepth
    };
    m_CommandList->RSSetViewports(1, &dxViewport);
}

void DX12CommandQueue::SetScissor(Scissor scissor) {
    D3D12_RECT dxScissor = {
        .left = scissor.Left,
        .top = scissor.Top,
        .right = scissor.Right,
        .bottom = scissor.Bottom
    };

    m_CommandList->RSSetScissorRects(1, &dxScissor);
}

void DX12CommandQueue::DrawInstaned(uint32_t VertexCountPerInstance, uint32_t InstanceCount,
        uint32_t StartVertexLocation, uint32_t StartInstanceLocation) {
    m_CommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void DX12CommandQueue::Flush() {
    SubmitCommandList();
    AcquireCommandAllocator();
}

void DX12CommandQueue::ReleaseResource(ReleaseResourceWrapper* resource) {
    m_ReleaseManager.ReleaseResource(resource);
}

void DX12CommandQueue::EndFrame() {
    uint64_t completedValue = m_Fence->GetCompletedValue();
    m_CommandAllocatorPool.Poll(completedValue);
    m_ReleaseManager.DiscardResources(completedValue);
}

void DX12CommandQueue::AcquireCommandAllocator() {
    m_CommandAllocator = m_CommandAllocatorPool.AcquireCommandAllocator();
    m_CommandList->Reset(m_CommandAllocator, nullptr);
    m_CommandAllocatorNumber++;
}

void DX12CommandQueue::SubmitCommandList() {
    m_CommandList->Close();
    Signal(m_Fence, m_CommandAllocatorNumber);

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

    m_CommandAllocatorPool.ReleaseCommandAllocator(m_CommandAllocator, m_CommandAllocatorNumber);
    m_ReleaseManager.DiscardStaleResources(m_CommandAllocatorNumber);
}

} // dx
