//
// Created by alan on 08/08/2026.
//

#include "Backend/DX12/DX12Swapchain.h"
#include "Backend/DX12/DX12Device.h"
#include <spdlog/spdlog.h>

namespace dx {

DX12Swapchain::DX12Swapchain(WindowInfo window, DX12Device* device, DX12CommandQueue* queue) {
    m_Device = device;
    m_Queue = queue;

    m_Window = window;

    RECT rect;
    GetClientRect(static_cast<HWND>(m_Window.Window), &rect);

    m_CurrentWidth = rect.right - rect.left;
    m_CurrentHeight = rect.bottom - rect.top;

    DXGI_MODE_DESC bufferMode = {
        .Format = DXGI_FORMAT_B8G8R8A8_UNORM
    };

    DXGI_SAMPLE_DESC sampleDesc = {
        .Count = 1,
        .Quality = 0,
    };

    DXGI_SWAP_CHAIN_DESC swapChainDesc = {
        .BufferDesc = bufferMode,
        .SampleDesc = sampleDesc,
        .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
        .BufferCount = 3,
        .OutputWindow = static_cast<HWND>(m_Window.Window),
        .Windowed = true,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD
    };

    IDXGISwapChain* tempSwapChain = nullptr;
    HRESULT hr = m_Device->GetDXGIFactory()->CreateSwapChain(m_Queue->GetDX12CommandQueue(), &swapChainDesc, &tempSwapChain);
    m_SwapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);
    m_CurrentImage = m_SwapChain->GetCurrentBackBufferIndex();

    m_Textures.resize(3);
    for (int i = 0; i < 3; i++) {
        ID3D12Resource* resource = nullptr;
        m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&resource));
        TextureDesc desc = {
            .Width = m_CurrentWidth,
            .Height = m_CurrentHeight,
            .MipLevels = 1,
            .ArrayLayers = 1,
            .Samples = 1,
            .Format = TextureFormat::B8G8R8A8_UNORM,
            .Type = TextureType::Texture2D,
            .BindFlags = TEXTURE_BIND_RENDER_TARGET
        };
        m_Textures[i] = new DX12Texture(desc, resource, m_Device);
    }

    m_Fence = new DX12Fence(m_Device);
    m_FrameFenceValues.resize(3);
    for (int i = 0; i < 3; i++) {
        m_FrameFenceValues[i] = 0;
    }

    spdlog::info("DX12Swapchain Created.");
}

DX12Swapchain::~DX12Swapchain() {
    m_FenceValue++;
    m_Queue->GetDX12CommandQueue()->Signal(m_Fence->GetDX12Fence(), m_FenceValue);
    m_Fence->Wait(m_FenceValue);
    if (m_Fence) {
        delete m_Fence;
    }
    if (m_SwapChain) {
        for (int i = 0; i < m_Textures.size(); i++) {
            delete m_Textures[i];
        }
        m_SwapChain->Release();
    }

    spdlog::info("DX12Swapchain Destroyed.");
}

void DX12Swapchain::Resize() {
    RECT rect;
    GetClientRect(static_cast<HWND>(m_Window.Window), &rect);
    m_CurrentWidth = rect.right - rect.left;
    m_CurrentHeight = rect.bottom - rect.top;

    m_FenceValue++;
    m_Queue->Flush();
    m_Queue->GetDX12CommandQueue()->Signal(m_Fence->GetDX12Fence(), m_FenceValue);
    m_Fence->Wait(m_FenceValue);
    for (int i = 0; i < 3; i++) {
        delete m_Textures[i];
    }
    m_Queue->Flush();
    m_Queue->EndFrame();

    m_SwapChain->ResizeBuffers(3, m_CurrentWidth, m_CurrentHeight, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
    for (int i = 0; i < 3; i++) {
        ID3D12Resource* resource = nullptr;
        m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&resource));
        TextureDesc desc = {
            .Width = m_CurrentWidth,
            .Height = m_CurrentHeight,
            .MipLevels = 1,
            .ArrayLayers = 1,
            .Samples = 1,
            .Format = TextureFormat::B8G8R8A8_UNORM,
            .Type = TextureType::Texture2D,
            .BindFlags = TEXTURE_BIND_RENDER_TARGET
        };
        m_Textures[i] = new DX12Texture(desc, resource, m_Device);
    }

    m_CurrentImage = m_SwapChain->GetCurrentBackBufferIndex();
}

Texture* DX12Swapchain::GetCurrentBackBuffer() {
    return m_Textures[m_CurrentImage];
}

void DX12Swapchain::Present() {
    m_FrameFenceValues[m_CurrentFrame] = ++m_FenceValue;
    m_Queue->Flush();
    m_Queue->GetDX12CommandQueue()->Signal(m_Fence->GetDX12Fence(), m_FrameFenceValues[m_CurrentFrame]);
    m_SwapChain->Present(1, 0);
    m_CurrentFrame = (m_CurrentFrame + 1) % 3;
    m_CurrentImage = m_SwapChain->GetCurrentBackBufferIndex();

    m_Fence->Wait(m_FrameFenceValues[m_CurrentFrame]);

    RECT rect;
    GetClientRect(static_cast<HWND>(m_Window.Window), &rect);
    if (m_CurrentWidth != rect.right - rect.left || m_CurrentHeight != rect.bottom - rect.top) {
        Resize();
    }
}

} // dx