//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_DX12SWAPCHAIN_H
#define DERANGED_RHI_DX12SWAPCHAIN_H

#include "Swapchain.h"
#include <GLFW/glfw3.h>
#include <dxgi1_4.h>

class DX12Device;

class DX12Swapchain : public Swapchain {
public:
    DX12Swapchain(DX12Device* device);
    ~DX12Swapchain() override;

    void UpdateWindow() override;
    bool WindowShouldClose() override;
    void Present() override;

private:
    DX12Device* m_Device = nullptr;
    GLFWwindow* m_Window = nullptr;
    IDXGISwapChain3* m_SwapChain = nullptr;

};

#endif //DERANGED_RHI_DX12SWAPCHAIN_H
