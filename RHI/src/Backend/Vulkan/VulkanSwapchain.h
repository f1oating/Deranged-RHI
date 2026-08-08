//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_VULKANSWAPCHAIN_H
#define DERANGED_RHI_VULKANSWAPCHAIN_H

#include <volk.h>
#include "Swapchain.h"
#include <GLFW/glfw3.h>
#include "Backend/Vulkan/VulkanCommandQueue.h"

class VulkanDevice;

class VulkanSwapchain : public Swapchain {
public:
    VulkanSwapchain(VulkanCommandQueue* queue, VulkanDevice* device);
    ~VulkanSwapchain() override;

    void UpdateWindow() override;
    bool WindowShouldClose() override;
    void Present() override;

private:
    void CreateWindow();
    void CreateSurface();
    void CheckQueueSupport();
    void CreateSwapchain();
    void DestroySwapchain();
    void DestroySurface();
    void DestroyWindow();

private:
    GLFWwindow* m_Window = nullptr;
    VulkanDevice* m_Device = nullptr;
    VulkanCommandQueue* m_Queue = nullptr;
    VkSurfaceKHR m_Surface = nullptr;
    VkSwapchainKHR m_SwapChain = nullptr;

};

#endif //DERANGED_RHI_VULKANSWAPCHAIN_H
