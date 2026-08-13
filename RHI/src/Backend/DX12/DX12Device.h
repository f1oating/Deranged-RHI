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

    void ReleaseResource(ReleaseResourceBase* resource);

    IDXGIFactory3* GetDXGIFactory() const { return m_Factory;}
    ID3D12Device4* GetDX12Device() const { return m_Device; }

private:
    ID3D12Debug3* m_Debug = nullptr;
    IDXGIFactory3* m_Factory = nullptr;
    ID3D12Device4* m_Device = nullptr;
    DWORD m_CallbackCookie = 0;
    ID3D12InfoQueue1* m_DebugQueue = nullptr;
    DX12CommandQueue* m_CommandQueue = nullptr;
    RingBuffer m_RingBuffer;

};

#endif //DERANGED_RHI_DX12DEVICE_H
