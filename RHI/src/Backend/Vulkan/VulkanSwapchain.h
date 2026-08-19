//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_VULKANSWAPCHAIN_H
#define DERANGED_RHI_VULKANSWAPCHAIN_H

#include <volk.h>
#include "Swapchain.h"
#include <GLFW/glfw3.h>
#include "Backend/Vulkan/VulkanCommandQueue.h"
#include "Backend/Vulkan/VulkanFence.h"
#include "Backend/Vulkan/VulkanResource.h"

namespace vk {

class VulkanDevice;

class VulkanSwapchain : public Swapchain {
public:
    VulkanSwapchain(VulkanCommandQueue* queue, VulkanDevice* device);
    ~VulkanSwapchain() override;

    Texture* GetCurrentBackBuffer() override;

    void UpdateWindow() override;
    bool WindowShouldClose() override;
    void Present() override;

private:
    void AcquireImage();

    void CreateWindow();
    void CreateSurface();
    void CheckQueueSupport();
    void CreateSwapchain();
    void CreateSync();
    void DestroySync();
    void DestroySwapchain();
    void DestroySurface();
    void DestroyWindow();

private:
    GLFWwindow* m_Window = nullptr;
    VulkanDevice* m_Device = nullptr;
    VulkanCommandQueue* m_Queue = nullptr;
    VkSurfaceKHR m_Surface = nullptr;
    VkSwapchainKHR m_SwapChain = nullptr;
    std::vector<VulkanTexture*> m_Textures;

    std::vector<VkSemaphore> m_AcquireSemaphores;
    std::vector<VkSemaphore> m_RenderSemaphores;
    std::vector<uint64_t> m_FrameFenceValues;
    VulkanFence* m_Fence = nullptr;
    uint64_t m_FenceValue = 0;
    uint64_t m_CurrentFrame = 0;
    uint32_t m_ImageIndex = 0;

};

} // vk

#endif //DERANGED_RHI_VULKANSWAPCHAIN_H
