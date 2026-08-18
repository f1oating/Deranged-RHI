//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_DX12SWAPCHAIN_H
#define DERANGED_RHI_DX12SWAPCHAIN_H

#include "Swapchain.h"
#include <GLFW/glfw3.h>
#include <dxgi1_4.h>
#include "Backend/DX12/DX12Fence.h"
#include <vector>

namespace dx {

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
    std::vector<ID3D12Resource*> m_Textures;
    uint32_t m_CurrentWidth = 0;
    uint32_t m_CurrentHeight = 0;
    DX12Fence* m_Fence = nullptr;
    uint64_t m_FenceValue = 0;
    std::vector<uint64_t> m_FrameFenceValues;
    uint64_t m_CurrentFrame = 0;

};

} // dx

#endif //DERANGED_RHI_DX12SWAPCHAIN_H
