//
// Created by alan on 08/08/2026.
//

#include "Backend/DX12/DX12CommandQueue.h"
#include "Backend/DX12/DX12Device.h"
#include "Backend/DX12/DX12Resource.h"
#include <spdlog/spdlog.h>

namespace dx {

DX12CommandQueue::DX12CommandQueue(DX12Device* device) {
    m_Device = device;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    HRESULT hr = m_Device->GetDX12Device()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_Queue));
    m_CommandAllocatorPool = std::make_unique<CommandAllocatorPool>(m_Device->GetDX12Device());
    m_DescriptorsStateManager = std::make_unique<DescriptorsStateManager>(m_Device->GetDX12Device());

    m_Fence = new DX12Fence(m_Device);
    hr = m_Device->GetDX12Device()->CreateCommandList1(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        D3D12_COMMAND_LIST_FLAG_NONE, IID_PPV_ARGS(&m_CommandList));

    AcquireCommandAllocator();

    spdlog::info("DX12CommandQueue Created.");
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
    m_DescriptorsStateManager.reset();
    m_CommandAllocatorPool.reset();
    if (m_Queue) {
        m_Queue->Release();
    }

    spdlog::info("DX12CommandQueue Destroyed.");
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

    m_DescriptorsStateManager->SetState(dxGraphicsPipelineState->GetDescriptorsState());
    m_GraphicsPipeline = dxGraphicsPipelineState;
    m_GraphicsPipelineBound = false;
}

void DX12CommandQueue::SetViewport(Viewport viewport) {
    m_Viewport = viewport;
    m_ViewportBound = false;
}

void DX12CommandQueue::SetScissor(Scissor scissor) {
    m_Scissor = scissor;
    m_ScissorBound = false;
}

void DX12CommandQueue::SetRenderTargets(std::vector<RenderTargetView*> rtvs) {
    m_RTVs.resize(rtvs.size());
    for (int i = 0; i < rtvs.size(); i++) {
        m_RTVs[i] = static_cast<DX12RenderTargetView*>(rtvs[i]);
    }
    m_RenderAttachmentsBound = false;
}

void DX12CommandQueue::SetDepthStencil(DepthStencilView *dsv) {
    m_DSV = static_cast<DX12DepthStencilView*>(dsv);
    m_RenderAttachmentsBound = false;
}

void DX12CommandQueue::ClearRenderTargets(float r, float g, float b, float a) {
    m_RTVsClearValue[0] = r;
    m_RTVsClearValue[1] = g;
    m_RTVsClearValue[2] = b;
    m_RTVsClearValue[3] = a;
    m_ShouldClearRTVs = true;
}

void DX12CommandQueue::ClearDepthStencil(float depth, uint8_t stencil) {
    m_DSVDepthClearValue = depth;
    m_DSVStencilClearValue = stencil;
    m_ShouldClearDSV = true;
}

void DX12CommandQueue::SetVertexBuffer(Buffer* buffer) {
    m_VertexBuffer = static_cast<DX12Buffer*>(buffer);
    m_VertexBufferBound = false;
}

void DX12CommandQueue::SetIndexBuffer(Buffer* buffer) {
    m_IndexBuffer = static_cast<DX12Buffer*>(buffer);
    m_IndexBufferBound = false;
}

void DX12CommandQueue::SetConstantBuffer(std::string name, Buffer* buffer) {
    DX12Buffer* dxBuffer = static_cast<DX12Buffer*>(buffer);
    D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {
        .BufferLocation = dxBuffer->GetDX12Resource()->GetGPUVirtualAddress() + dxBuffer->GetOffset(),
        .SizeInBytes = (uint32_t)dxBuffer->GetDesc().Size,
    };
    m_DescriptorsStateManager->SetCBV(name, desc);
}

void DX12CommandQueue::SetTexture(std::string name, ShaderResourceView* textureView) {
    DX12ShaderResourceView* dxTextureView = static_cast<DX12ShaderResourceView*>(textureView);

    m_DescriptorsStateManager->SetSRV(name, dxTextureView->GetDXTexture()->GetDX12Resource(), dxTextureView->GetDXView());
}

void DX12CommandQueue::SetSampler(std::string name, Sampler* sampler) {
    DX12Sampler* dxSampler = static_cast<DX12Sampler*>(sampler);

    m_DescriptorsStateManager->SetSampler(name, dxSampler->GetDXSampler());
}

void DX12CommandQueue::DrawInstanced(uint32_t vertexCount, uint32_t instanceCount,
        uint32_t startVertex, uint32_t startInstance) {
    BoundDirtyResources();
    ClearRenderAttachmentsIfNeeded();

    auto [allocation, samplerAllocation] = m_DescriptorsStateManager->WriteAndAllocate(m_CommandAllocatorNumber);
    if (m_GraphicsPipeline->HaveResources()) {
        m_CommandList->SetGraphicsRootDescriptorTable(0, allocation.GetGPUHandle(0));
    }
    if (m_GraphicsPipeline->HaveSamplers()) {
        m_CommandList->SetGraphicsRootDescriptorTable(1, samplerAllocation.GetGPUHandle(0));
    }

    m_CommandList->DrawInstanced(vertexCount, instanceCount, startVertex, startInstance);
}

void DX12CommandQueue::DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount,
    uint32_t startIndex, uint32_t vertexOffset, uint32_t startInstance) {
    BoundDirtyResources();
    ClearRenderAttachmentsIfNeeded();

    auto [allocation, samplerAllocation] = m_DescriptorsStateManager->WriteAndAllocate(m_CommandAllocatorNumber);
    if (m_GraphicsPipeline->HaveResources()) {
        m_CommandList->SetGraphicsRootDescriptorTable(0, allocation.GetGPUHandle(0));
    }
    if (m_GraphicsPipeline->HaveSamplers()) {
        m_CommandList->SetGraphicsRootDescriptorTable(1, samplerAllocation.GetGPUHandle(0));
    }

    m_CommandList->DrawIndexedInstanced(indexCount, instanceCount, startIndex, vertexOffset, startInstance);
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

void DX12CommandQueue::CopyToTexture(Texture* dst, uint64_t size, void* data) {
    DX12Texture* dxDst = static_cast<DX12Texture*>(dst);

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
    MarkResourcesDirty();
}

void DX12CommandQueue::ReleaseResource(ReleaseResourceWrapper* resource) {
    m_ReleaseManager.ReleaseResource(resource);
}

void DX12CommandQueue::EndFrame() {
    uint64_t completedValue = m_Fence->GetCompletedValue();
    m_CommandAllocatorPool->Poll(completedValue);
    m_DescriptorsStateManager->FreeFrames(completedValue);
    m_ReleaseManager.DiscardResources(completedValue);
}

void DX12CommandQueue::MarkResourcesDirty() {
    m_GraphicsPipelineBound = false;
    m_ViewportBound = false;
    m_ScissorBound = false;
    m_VertexBufferBound = false;
    m_IndexBufferBound = false;
}

void DX12CommandQueue::BoundDirtyResources() {
    if (!m_GraphicsPipelineBound) {
        m_CommandList->SetGraphicsRootSignature(m_GraphicsPipeline->GetRootSignature());
        m_CommandList->SetPipelineState(m_GraphicsPipeline->GetPipelineState());
        m_CommandList->IASetPrimitiveTopology(ToD3D12PrimitiveTopology(m_GraphicsPipeline->GetDesc().PrimitiveTopology));
    }
    if (!m_ViewportBound) {
        D3D12_VIEWPORT dxViewport = {
            .TopLeftX = m_Viewport.TopLeftX,
            .TopLeftY = m_Viewport.TopLeftY,
            .Width = m_Viewport.Width,
            .Height = m_Viewport.Height,
            .MinDepth = m_Viewport.MinDepth,
            .MaxDepth = m_Viewport.MaxDepth
        };
        m_CommandList->RSSetViewports(1, &dxViewport);
    }
    if (!m_ScissorBound) {
        D3D12_RECT dxScissor = {
            .left = m_Scissor.Left,
            .top = m_Scissor.Top,
            .right = m_Scissor.Right,
            .bottom = m_Scissor.Bottom
        };

        m_CommandList->RSSetScissorRects(1, &dxScissor);
    }
    if (!m_VertexBufferBound) {
        D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {
            .BufferLocation = m_VertexBuffer->GetDX12Resource()->GetGPUVirtualAddress(),
            .SizeInBytes = (uint32_t)m_VertexBuffer->GetDesc().Size,
            .StrideInBytes = m_VertexBuffer->GetDesc().Stride
        };
        m_CommandList->IASetVertexBuffers(0, 1, &vertexBufferView);
    }
    if (!m_IndexBufferBound) {
        D3D12_INDEX_BUFFER_VIEW indexBufferView = {
            .BufferLocation = m_IndexBuffer->GetDX12Resource()->GetGPUVirtualAddress(),
            .SizeInBytes = (uint32_t)m_IndexBuffer->GetDesc().Size,
            .Format = DXGI_FORMAT_R32_UINT
        };
        m_CommandList->IASetIndexBuffer(&indexBufferView);
    }
    if (!m_RenderAttachmentsBound) {
        std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rtvHandles;
        for (auto* view : m_RTVs) {
            rtvHandles.push_back(view->GetAllocation().GetCPUHandle(0));
        }
        D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = m_DSV->GetAllocation().GetCPUHandle(0);
        m_CommandList->OMSetRenderTargets(rtvHandles.size(),
        rtvHandles.data(), false, &dsvHandle);
    }
}

void DX12CommandQueue::ClearRenderAttachmentsIfNeeded() {
    if (m_ShouldClearRTVs) {
        for (auto rtv : m_RTVs) {
            m_CommandList->ClearRenderTargetView(rtv->GetAllocation().GetCPUHandle(0), m_RTVsClearValue, 0, nullptr);
        }
        m_ShouldClearRTVs = false;
    }
    if (m_ShouldClearDSV) {
        m_CommandList->ClearDepthStencilView(m_DSV->GetAllocation().GetCPUHandle(0), m_DSV->GetDX12ClearFlags(),
            m_DSVDepthClearValue, m_DSVStencilClearValue, 0, nullptr);
        m_ShouldClearDSV = false;
    }
}

void DX12CommandQueue::AcquireCommandAllocator() {
    m_CommandAllocator = m_CommandAllocatorPool->AcquireCommandAllocator();
    m_CommandList->Reset(m_CommandAllocator, nullptr);
    ID3D12DescriptorHeap* heaps[2] = { m_DescriptorsStateManager->GetDX12Heap(), m_DescriptorsStateManager->GetDX12SamplerHeap() };
    m_CommandList->SetDescriptorHeaps(2, heaps);
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

    m_CommandAllocatorPool->ReleaseCommandAllocator(m_CommandAllocator, m_CommandAllocatorNumber);
    m_ReleaseManager.DiscardStaleResources(m_CommandAllocatorNumber);
}

} // dx
