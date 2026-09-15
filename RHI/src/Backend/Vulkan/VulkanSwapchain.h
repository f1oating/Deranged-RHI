//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_VULKANSWAPCHAIN_H
#define DERANGED_RHI_VULKANSWAPCHAIN_H

#include <volk.h>
#include "Swapchain.h"
#include "Backend/Vulkan/VulkanCommandQueue.h"
#include "Backend/Vulkan/VulkanFence.h"
#include "Backend/Vulkan/VulkanResource.h"

namespace vk {

class VulkanDevice;

class VulkanSwapchain : public Swapchain {
public:
    VulkanSwapchain(WindowInfo window, VulkanCommandQueue* queue, VulkanDevice* device);
    ~VulkanSwapchain() override;

    Texture* GetCurrentBackBuffer() override;

    void Present() override;

private:
    void AcquireImage();

    void CreateSurface();
    void CheckQueueSupport();
    void CreateSwapchain();
    void CreateSync();

    void DestroySync();
    void DestroySwapchain();
    void DestroySurface();

private:
    WindowInfo m_Window;

    VulkanDevice* m_Device = nullptr;
    VulkanCommandQueue* m_Queue = nullptr;

    VkSurfaceKHR m_Surface = nullptr;
    VkSwapchainKHR m_SwapChain = nullptr;

    std::vector<VulkanTexture*> m_Textures;

    VulkanFence* m_Fence = nullptr;
    std::vector<uint64_t> m_FrameFenceValues;
    std::vector<VkSemaphore> m_AcquireSemaphores;
    std::vector<VkSemaphore> m_RenderSemaphores;

    uint64_t m_FenceValue = 0;
    uint64_t m_CurrentFrame = 0;
    uint32_t m_ImageIndex = 0;

};

} // vk

#endif //DERANGED_RHI_VULKANSWAPCHAIN_H
