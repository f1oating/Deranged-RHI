//
// Created by alan on 08/08/2026.
//

#include "Backend/DX12/DX12Device.h"
#include "Backend/DX12/DX12Swapchain.h"
#include <iostream>
#include <ostream>
#include <GLFW/glfw3.h>

void DebugCallback(
    D3D12_MESSAGE_CATEGORY Category,
    D3D12_MESSAGE_SEVERITY Severity,
    D3D12_MESSAGE_ID ID,
    LPCSTR pDescription,
    void* pContext) {
    std::cout << "[" << pDescription << "]" << std::endl;
}

DX12Device::DX12Device() {
    glfwInit();

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

    m_CommandQueue = new DX12CommandQueue(this);
}

DX12Device::~DX12Device() {
    m_DebugQueue->UnregisterMessageCallback(m_CallbackCookie);
    if (m_CommandQueue) {
        delete m_CommandQueue;
    }
    if (m_Device) {
        m_Device->Release();
    }
    if (m_Factory) {
        m_Factory->Release();
    }
    if (m_Debug) {
        m_Debug->Release();
    }
    glfwTerminate();
}

void DX12Device::EndFrame() {
    m_CommandQueue->EndFrame();
}

CommandQueue* DX12Device::GetCommandQueue() {
    return m_CommandQueue;
}

Swapchain* DX12Device::CreateSwapchain() {
    return new DX12Swapchain(this);
}

void DX12Device::ReleaseResource(ReleaseResourceBase* resource) {
    m_CommandQueue->ReleaseResource(new ReleaseResourceWrapper(resource, 1));
}