//
// Created by alan on 08/08/2026.
//

#include "Backend/DX12/DX12CommandQueue.h"
#include "Backend/DX12/DX12Device.h"
#include "Backend/DX12/DX12Resource.h"

namespace dx {

DX12CommandQueue::DX12CommandQueue(DX12Device* device) {
    m_Device = device;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    HRESULT hr = m_Device->GetDX12Device()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_Queue));
    m_CommandAllocatorPool.Init(m_Device->GetDX12Device());
    m_DescriptorsStateManager.Init(m_Device->GetDX12Device());

    m_Fence = new DX12Fence(m_Device);
    hr = m_Device->GetDX12Device()->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
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
    m_DescriptorsStateManager.Shutdown();
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
    m_CommandList->IASetPrimitiveTopology(ToD3D12PrimitiveTopology(dxGraphicsPipelineState->GetDesc().PrimitiveTopology));

    m_BoundPipeline = dxGraphicsPipelineState;
    m_DescriptorsStateManager.SetState(m_BoundPipeline->GetDescriptorsState());
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

void DX12CommandQueue::SetBlendConstants(float r, float g, float b, float a) {
    float rgba[] = { r, g, b, a };
    m_CommandList->OMSetBlendFactor(rgba);
}

void DX12CommandQueue::Barrier(uint32_t srcStage, uint32_t dstStage,
    std::vector<BufferBarrier> bufBarriers, std::vector<TextureBarrier> texBarriers) {
    std::vector<D3D12_TEXTURE_BARRIER> textureBarriers;
    std::vector<D3D12_BUFFER_BARRIER> bufferBarriers;

    for (int i = 0; i < texBarriers.size(); i++) {
        DX12Texture* dxTexture = static_cast<DX12Texture*>(texBarriers[i].Tex);

        D3D12_BARRIER_SUBRESOURCE_RANGE range = {
            .IndexOrFirstMipLevel = 0,
            .NumMipLevels = 1,
            .FirstArraySlice = 0,
            .NumArraySlices = 1,
            .FirstPlane = 0,
            .NumPlanes = 1
        };

        D3D12_TEXTURE_BARRIER textureBarrier = {
            .SyncBefore = ToD3D12BarrierSync(srcStage),
            .SyncAfter = ToD3D12BarrierSync(dstStage),
            .AccessBefore = ToD3D12BarrierAccess(texBarriers[i].SrcAccessFlags),
            .AccessAfter = ToD3D12BarrierAccess(texBarriers[i].DstAccessFlags),
            .LayoutBefore = ToD3D12BarrierLayout(dxTexture->GetResourceLayout()),
            .LayoutAfter = ToD3D12BarrierLayout(texBarriers[i].Layout),
            .pResource = dxTexture->GetDX12Resource(),
            .Subresources = range
        };

        textureBarriers.push_back(textureBarrier);
        dxTexture->SetResourceLayout(texBarriers[i].Layout);
    }

    for (int i = 0; i < bufBarriers.size(); i++) {
        DX12Buffer* dxBuffer = static_cast<DX12Buffer*>(bufBarriers[i].Buf);

        D3D12_BARRIER_SUBRESOURCE_RANGE range = {
            .IndexOrFirstMipLevel = 0,
            .NumMipLevels = 1,
            .FirstArraySlice = 0,
            .NumArraySlices = 1,
            .FirstPlane = 0,
            .NumPlanes = 1
        };

        D3D12_BUFFER_BARRIER bufferBarrier = {
            .SyncBefore = ToD3D12BarrierSync(srcStage),
            .SyncAfter = ToD3D12BarrierSync(dstStage),
            .AccessBefore = ToD3D12BarrierAccess(bufBarriers[i].SrcAccessFlags),
            .AccessAfter = ToD3D12BarrierAccess(bufBarriers[i].DstAccessFlags),
            .pResource = dxBuffer->GetDX12Resource(),
            .Offset = 0,
            .Size = dxBuffer->GetDesc().Size
        };

        bufferBarriers.push_back(bufferBarrier);
    }

    D3D12_BARRIER_GROUP barrierGroups[2];
    barrierGroups[0].Type = D3D12_BARRIER_TYPE_TEXTURE;
    barrierGroups[0].NumBarriers = (uint32_t)textureBarriers.size();
    barrierGroups[0].pTextureBarriers = textureBarriers.data();

    barrierGroups[1].Type = D3D12_BARRIER_TYPE_BUFFER;
    barrierGroups[1].NumBarriers = (uint32_t)bufferBarriers.size();
    barrierGroups[1].pBufferBarriers = bufferBarriers.data();

    m_CommandList->Barrier(2, barrierGroups);
}

void DX12CommandQueue::SetRenderTargets(std::vector<TextureView*> rtvs) {
    for (auto rtv : rtvs) {
        DX12TextureView* dxRTV = static_cast<DX12TextureView*>(rtv);
        m_RTVs.push_back(dxRTV->GetDescriptor().GetCPUHandle(0));
    }

    m_CommandList->OMSetRenderTargets(m_RTVs.size(),
        m_RTVs.data(), false, nullptr);
}

void DX12CommandQueue::ClearRenderTargets(float r, float g, float b, float a) {
    const float clear[] = { r, g, b, a };
    for (auto rtv : m_RTVs) {
        m_CommandList->ClearRenderTargetView(rtv, clear, 0, nullptr);
    }
}

void DX12CommandQueue::SetVertexBuffer(Buffer* buffer, uint32_t stride) {
    DX12Buffer* dxBuffer = static_cast<DX12Buffer*>(buffer);

    D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {
        .BufferLocation = dxBuffer->GetDX12Resource()->GetGPUVirtualAddress(),
        .SizeInBytes = (uint32_t)dxBuffer->GetDesc().Size,
        .StrideInBytes = stride
    };
    m_CommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
}

void DX12CommandQueue::SetConstantBuffer(std::string name, Buffer* buffer) {
    DX12Buffer* dxBuffer = static_cast<DX12Buffer*>(buffer);
    D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {
        .BufferLocation = dxBuffer->GetDX12Resource()->GetGPUVirtualAddress(),
        .SizeInBytes = (uint32_t)dxBuffer->GetDesc().Size,
    };
    m_DescriptorsStateManager.SetCBV(m_BoundPipeline->GetDescriptorOffset(name), desc);
}

void DX12CommandQueue::DrawInstansed(uint32_t VertexCountPerInstance, uint32_t InstanceCount,
        uint32_t StartVertexLocation, uint32_t StartInstanceLocation) {
    DescriptorHeapAllocation allocation = m_DescriptorsStateManager.WriteAndAllocate(m_CommandAllocatorNumber);
    m_CommandList->SetGraphicsRootDescriptorTable(0, allocation.GetGPUHandle(0));
    m_CommandList->DrawInstanced(VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void DX12CommandQueue::CopyToBuffer(Buffer* dst, uint64_t size, void* data) {
    DX12Buffer* dxDst = static_cast<DX12Buffer*>(dst);

    ID3D12Resource* src = nullptr;

    D3D12_HEAP_PROPERTIES heapProps = {
        .Type = D3D12_HEAP_TYPE_UPLOAD
    };

    D3D12_RESOURCE_DESC1 resourceDesc = {
        .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
        .Width =  size,
        .Height = 1,
        .DepthOrArraySize = 1,
        .MipLevels = 1,
        .SampleDesc = { 1, 0 },
        .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR
    };

    m_Device->GetDX12Device()->CreateCommittedResource3(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
        D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, nullptr, 0, nullptr, IID_PPV_ARGS(&src));

    void* mapped = nullptr;
    src->Map(0, nullptr, &mapped);
    memcpy(mapped, data, size);
    src->Unmap(0, nullptr);

    m_CommandList->CopyBufferRegion(dxDst->GetDX12Resource(), 0, src, 0, size);

    ReleaseResource(new ReleaseResourceWrapper(new BufferReleaseResource(src)));
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
    m_DescriptorsStateManager.FreeFrames(completedValue);
    m_ReleaseManager.DiscardResources(completedValue);
}

void DX12CommandQueue::AcquireCommandAllocator() {
    m_CommandAllocator = m_CommandAllocatorPool.AcquireCommandAllocator();
    m_CommandList->Reset(m_CommandAllocator, nullptr);
    ID3D12DescriptorHeap* heap = m_DescriptorsStateManager.GetDX12Heap();
    m_CommandList->SetDescriptorHeaps(1, &heap);
    m_CommandAllocatorNumber++;
}

void DX12CommandQueue::SubmitCommandList() {
    HRESULT hr = m_CommandList->Close();
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
    m_RTVs.clear();

    m_CommandAllocatorPool.ReleaseCommandAllocator(m_CommandAllocator, m_CommandAllocatorNumber);
    m_ReleaseManager.DiscardStaleResources(m_CommandAllocatorNumber);
}

} // dx
