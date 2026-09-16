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
    DX12Device(DeviceDesc desc);
    ~DX12Device() override;

    void EndFrame() override;

    CommandQueue* GetCommandQueue();
    Swapchain* CreateSwapchain(WindowInfo window) override;
    GraphicsPipelineState* CreateGraphicsPipelineState(GraphicsPipelineDesc desc) override;
    Texture* CreateTexture(TextureDesc desc) override;
    RenderTargetView* CreateRenderTargetView(Texture* texture) override;
    DepthStencilView* CreateDepthStencilView(Texture* texture) override;
    ShaderResourceView* CreateShaderResourceView(Texture* texture) override;
    Buffer* CreateBuffer(BufferDesc desc) override;
    Sampler* CreateSampler(SamplerDesc desc) override;

    DeviceDesc GetDesc() override;

    void ReleaseResource(ReleaseResourceBase* resource);

    IDXGIFactory3* GetDXGIFactory() const { return m_Factory;}
    ID3D12Device10* GetDX12Device() const { return m_Device; }
    DescriptorHeap* GetRTVAllocator() { return m_RTVAllocator.get(); }
    DescriptorHeap* GetDSVAllocator() { return m_DSVAllocator.get(); }
    RingBuffer* GetRingBuffer() { return m_RingBuffer.get(); }

private:
    DeviceDesc m_Desc;

    ID3D12Debug3* m_Debug = nullptr;
    DWORD m_CallbackCookie = 0;

    IDXGIFactory3* m_Factory = nullptr;
    ID3D12Device10* m_Device = nullptr;
    ID3D12InfoQueue1* m_DebugQueue = nullptr;
    DX12CommandQueue* m_CommandQueue = nullptr;

    std::unique_ptr<RingBuffer> m_RingBuffer = nullptr;
    std::unique_ptr<DescriptorHeap> m_RTVAllocator = nullptr;
    std::unique_ptr<DescriptorHeap> m_DSVAllocator = nullptr;

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
