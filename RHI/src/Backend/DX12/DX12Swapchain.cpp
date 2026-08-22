//
// Created by alan on 08/08/2026.
//

#include "Backend/DX12/DX12Swapchain.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include "Backend/DX12/DX12Device.h"

namespace dx {

DX12Swapchain::DX12Swapchain(DX12Device* device) {
    m_Device = device;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_Window = glfwCreateWindow(800, 600, "RHI", nullptr, nullptr);

    m_CurrentWidth = 800;
    m_CurrentHeight = 600;

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
        .OutputWindow = glfwGetWin32Window(m_Window),
        .Windowed = true,
        .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD
    };

    IDXGISwapChain* tempSwapChain = nullptr;
    DX12CommandQueue* dxCommandQueue = static_cast<DX12CommandQueue*>(m_Device->GetCommandQueue());
    HRESULT hr = m_Device->GetDXGIFactory()->CreateSwapChain(dxCommandQueue->GetDX12CommandQueue(), &swapChainDesc, &tempSwapChain);
    m_SwapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);
    m_CurrentImage = m_SwapChain->GetCurrentBackBufferIndex();

    m_Textures.resize(3);
    for (int i = 0; i < 3; i++) {
        ID3D12Resource* resource = nullptr;
        m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&resource));
        TextureDesc desc = {
            .Width = 800,
            .Height = 600,
            .MipLevels = 1,
            .ArrayLayers = 1,
            .Samples = 1,
            .Format = TextureFormat::B8G8R8A8_UNORM,
            .Type = TextureType::Texture2D,
            .Bind = RESOURCE_BIND_RENDER_TARGET,
            .Usage = ResourceUsage::Default
        };
        m_Textures[i] = new DX12Texture(desc, resource, m_Device);
    }

    m_Fence = new DX12Fence(m_Device);
    m_FrameFenceValues.resize(3);
    for (int i = 0; i < 3; i++) {
        m_FrameFenceValues[i] = 0;
    }
}

DX12Swapchain::~DX12Swapchain() {
    m_FenceValue++;
    DX12CommandQueue* dxCommandQueue = static_cast<DX12CommandQueue*>(m_Device->GetCommandQueue());
    dxCommandQueue->GetDX12CommandQueue()->Signal(m_Fence->GetDX12Fence(), m_FenceValue);
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
    if (m_Window) {
        glfwDestroyWindow(m_Window);
    }
}

Texture* DX12Swapchain::GetCurrentBackBuffer() {
    return m_Textures[m_CurrentImage];
}

void DX12Swapchain::UpdateWindow() {
    glfwPollEvents();
}
bool DX12Swapchain::WindowShouldClose() {
    return glfwWindowShouldClose(m_Window);
}

void DX12Swapchain::Present() {
    DX12CommandQueue* dxCommandQueue = static_cast<DX12CommandQueue*>(m_Device->GetCommandQueue());
    m_FrameFenceValues[m_CurrentFrame] = ++m_FenceValue;
    dxCommandQueue->Flush();
    dxCommandQueue->GetDX12CommandQueue()->Signal(m_Fence->GetDX12Fence(), m_FrameFenceValues[m_CurrentFrame]);
    m_SwapChain->Present(1, 0);
    m_CurrentFrame = (m_CurrentFrame + 1) % 3;
    m_CurrentImage = m_SwapChain->GetCurrentBackBufferIndex();

    m_Fence->Wait(m_FrameFenceValues[m_CurrentFrame]);

    int width, height;
    glfwGetWindowSize(m_Window, &width, &height);
    if (m_CurrentWidth != width || m_CurrentHeight != height) {
        m_FenceValue++;
        DX12CommandQueue* dxCommandQueue = static_cast<DX12CommandQueue*>(m_Device->GetCommandQueue());
        dxCommandQueue->Flush();
        dxCommandQueue->GetDX12CommandQueue()->Signal(m_Fence->GetDX12Fence(), m_FenceValue);
        m_Fence->Wait(m_FenceValue);
        for (int i = 0; i < 3; i++) {
            delete m_Textures[i];
        }
        dxCommandQueue->Flush();
        dxCommandQueue->EndFrame();
        m_SwapChain->ResizeBuffers(3, width, height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
        for (int i = 0; i < 3; i++) {
            ID3D12Resource* resource = nullptr;
            m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&resource));
            TextureDesc desc = {
                .Width = (uint32_t)width,
                .Height = (uint32_t)height,
                .MipLevels = 1,
                .ArrayLayers = 1,
                .Samples = 1,
                .Format = TextureFormat::B8G8R8A8_UNORM,
                .Type = TextureType::Texture2D,
                .Bind = RESOURCE_BIND_RENDER_TARGET,
                .Usage = ResourceUsage::Default
            };
            m_Textures[i] = new DX12Texture(desc, resource, m_Device);
        }
        m_CurrentWidth = width;
        m_CurrentHeight = height;
        m_CurrentImage = m_SwapChain->GetCurrentBackBufferIndex();
    }
}

} // dx