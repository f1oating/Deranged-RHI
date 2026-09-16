//
// Created by alan on 08/08/2026.
//

#include "Backend/DX12/DX12Device.h"
#include "Backend/DX12/DX12Swapchain.h"
#include <iostream>
#include "Backend/DX12/DX12Resource.h"
#include "Backend/DX12/DX12Pipeline.h"
#include <spdlog/spdlog.h>

namespace dx {

void DebugCallback(
    D3D12_MESSAGE_CATEGORY Category,
    D3D12_MESSAGE_SEVERITY Severity,
    D3D12_MESSAGE_ID ID,
    LPCSTR pDescription,
    void* pContext) {
    switch (Severity) {
        case D3D12_MESSAGE_SEVERITY_INFO:
        case D3D12_MESSAGE_SEVERITY_MESSAGE:
            spdlog::info(pDescription);
        case D3D12_MESSAGE_SEVERITY_WARNING:
            spdlog::warn(pDescription);
            return;
        case D3D12_MESSAGE_SEVERITY_ERROR:
            spdlog::error(pDescription);
            return;
        case D3D12_MESSAGE_SEVERITY_CORRUPTION:
            spdlog::critical(pDescription);
            return;
        default:
            spdlog::info(pDescription);
    }
}

DX12Device::DX12Device(DeviceDesc desc) {
    m_Desc = desc;

    HRESULT hr = D3D12GetDebugInterface(IID_PPV_ARGS(&m_Debug));
    m_Debug->EnableDebugLayer();
    m_Debug->SetEnableGPUBasedValidation(true);

    hr = CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_Factory));

    uint32_t adapterIndex = 0;
    IDXGIAdapter1* adapter;
    while (m_Factory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND) {
        DXGI_ADAPTER_DESC1 adapterDesc;
        adapter->GetDesc1(&adapterDesc);

        if (adapterDesc.Flags != DXGI_ADAPTER_FLAG_SOFTWARE) {
            break;
        }

        adapterIndex++;
    }

    hr = D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_12_2, IID_PPV_ARGS(&m_Device));
    m_Device->QueryInterface(IID_PPV_ARGS(&m_DebugQueue));

    m_DebugQueue->RegisterMessageCallback(DebugCallback, D3D12_MESSAGE_CALLBACK_FLAG_NONE, nullptr, &m_CallbackCookie);

    m_RingBuffer = std::make_unique<RingBuffer>(m_Device);
    m_RTVAllocator = std::make_unique<DescriptorHeap>(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 32);
    m_DSVAllocator = std::make_unique<DescriptorHeap>(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 32);

    m_CommandQueue = new DX12CommandQueue(this);

    spdlog::info("DX12Device Created.");
}

DX12Device::~DX12Device() {
    if (m_CommandQueue) {
        delete m_CommandQueue;
    }
    m_DSVAllocator.reset();
    m_RTVAllocator.reset();
    m_RingBuffer.reset();
    m_DebugQueue->UnregisterMessageCallback(m_CallbackCookie);
    if (m_Device) {
        m_Device->Release();
    }
    if (m_Factory) {
        m_Factory->Release();
    }
    if (m_Debug) {
        m_Debug->Release();
    }

    spdlog::info("DX12Device Destroyed.");
}

void DX12Device::EndFrame() {
    ReleaseResource(new RingBufferReleaseResource(m_RingBuffer.get(), m_RingBuffer->GetHead()));
    m_CommandQueue->EndFrame();
}

CommandQueue* DX12Device::GetCommandQueue() {
    return m_CommandQueue;
}

Swapchain* DX12Device::CreateSwapchain(WindowInfo window) {
    return new DX12Swapchain(window, this);
}

GraphicsPipelineState* DX12Device::CreateGraphicsPipelineState(GraphicsPipelineDesc desc) {
    return new DX12GraphicsPipelineState(desc, this);
}

Texture* DX12Device::CreateTexture(TextureDesc desc) {
    return new DX12Texture(desc, this);
}

RenderTargetView* DX12Device::CreateRenderTargetView(Texture* texture) {
    return new DX12RenderTargetView(static_cast<DX12Texture*>(texture), this);
}

DepthStencilView* DX12Device::CreateDepthStencilView(Texture* texture) {
    return new DX12DepthStencilView(static_cast<DX12Texture*>(texture), this);
}

ShaderResourceView* DX12Device::CreateShaderResourceView(Texture* texture) {
    return new DX12ShaderResourceView(static_cast<DX12Texture*>(texture), this);
}

Buffer* DX12Device::CreateBuffer(BufferDesc desc) {
    return new DX12Buffer(desc, this);
}

Sampler* DX12Device::CreateSampler(SamplerDesc desc) {
    return new DX12Sampler(desc, this);
}

DeviceDesc DX12Device::GetDesc() {
    return m_Desc;
}

void DX12Device::ReleaseResource(ReleaseResourceBase* resource) {
    m_CommandQueue->ReleaseResource(new ReleaseResourceWrapper(resource, 1));
}

} // dx