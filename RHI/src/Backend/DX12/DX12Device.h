//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_DX12DEVICE_H
#define DERANGED_RHI_DX12DEVICE_H

#include "Device.h"
#include <dxgi1_3.h>
#include <d3d12.h>
#include "Backend/DX12/DX12CommandQueue.h"
#include "Backend/DX12/Internal/RingBuffer.h"
#include "Backend/DX12/Internal/DescriptorHeap.h"

namespace dx {

class DX12Device : public Device {
public:
    DX12Device();
    ~DX12Device() override;

    void EndFrame() override;

    CommandQueue* GetCommandQueue();
    Swapchain* CreateSwapchain() override;
    GraphicsPipelineState* CreateGraphicsPipelineState(GraphicsPipelineDesc desc) override;
    Texture* CreateTexture(TextureDesc desc) override;
    TextureView* CreateTextureView(TextureViewDesc desc) override;
    Buffer* CreateBuffer(BufferDesc desc) override;

    void ReleaseResource(ReleaseResourceBase* resource);

    IDXGIFactory3* GetDXGIFactory() const { return m_Factory;}
    ID3D12Device10* GetDX12Device() const { return m_Device; }
    DescriptorHeap* GetRTVAllocator() { return &m_RTVAllocator; }
    DescriptorHeap* GetDSVAllocator() { return &m_DSVAllocator; }
    RingBuffer* GetRingBuffer() { return &m_RingBuffer; }

private:
    ID3D12Debug3* m_Debug = nullptr;
    IDXGIFactory3* m_Factory = nullptr;
    ID3D12Device10* m_Device = nullptr;
    DWORD m_CallbackCookie = 0;
    ID3D12InfoQueue1* m_DebugQueue = nullptr;
    DX12CommandQueue* m_CommandQueue = nullptr;
    RingBuffer m_RingBuffer;
    DescriptorHeap m_RTVAllocator;
    DescriptorHeap m_DSVAllocator;

};

struct RingBufferReleaseResource : ReleaseResourceBase {
    RingBuffer* Buffer;
    uint64_t Tail;

    RingBufferReleaseResource(RingBuffer* buffer, uint64_t tail)
        : Buffer(buffer), Tail(tail) {}

    void Destroy() override {
        Buffer->SetTail(Tail);
    }
};

} // dx

#endif //DERANGED_RHI_DX12DEVICE_H
