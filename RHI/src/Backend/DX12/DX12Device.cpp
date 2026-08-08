//
// Created by alan on 08/08/2026.
//

#include "Backend/DX12/DX12Device.h"
#include "Backend/DX12/DX12Swapchain.h"
#include <cstdint>
#include <GLFW/glfw3.h>

DX12Device::DX12Device() {
    glfwInit();
    HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&m_Factory));

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

    m_CommandQueue = new DX12CommandQueue(this);
}

DX12Device::~DX12Device() {
    if (m_CommandQueue) {
        delete m_CommandQueue;
    }
    if (m_Device) {
        m_Device->Release();
    }
    if (m_Factory) {
        m_Factory->Release();
    }
    glfwTerminate();
}

Swapchain* DX12Device::CreateSwapchain() {
    return new DX12Swapchain(this);
}