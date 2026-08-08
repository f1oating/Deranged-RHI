//
// Created by alan on 08/08/2026.
//

#include "Backend/DX12/DX12Swapchain.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "DX12Device.h"

DX12Swapchain::DX12Swapchain(DX12Device* device) {
    m_Device = device;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_Window = glfwCreateWindow(800, 600, "RHI", nullptr, nullptr);

    DXGI_MODE_DESC bufferMode = {
        .Format = DXGI_FORMAT_R8G8B8A8_UNORM
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
    HRESULT hr = m_Device->GetDXGIFactory()->CreateSwapChain(m_Device->GetCommandQueue()->GetDX12CommandQueue(), &swapChainDesc, &tempSwapChain);
    m_SwapChain = static_cast<IDXGISwapChain3*>(tempSwapChain);
}

DX12Swapchain::~DX12Swapchain() {
    if (m_SwapChain) {
        m_SwapChain->Release();
    }
    if (m_Window) {
        glfwDestroyWindow(m_Window);
    }
}

void DX12Swapchain::UpdateWindow() {
    glfwPollEvents();
}
bool DX12Swapchain::WindowShouldClose() {
    return glfwWindowShouldClose(m_Window);
}

void DX12Swapchain::Present() {

}