//
// Created by alan on 08/08/2026.
//

#include "Backend/Vulkan/VulkanSwapchain.h"
#include "Backend/Vulkan/VulkanDevice.h"
#include <vector>

namespace vk {

VulkanSwapchain::VulkanSwapchain(VulkanCommandQueue* queue, VulkanDevice* device) {
    m_Queue = queue;
    m_Device = device;

    CreateWindow();
    CreateSurface();
    CheckQueueSupport();
    CreateSwapchain();
    CreateSync();

    m_Fence = new VulkanFence(m_Device);
    m_FrameFenceValues.resize(3);
    for (uint32_t i = 0; i < 3; i++) {
        m_FrameFenceValues[i] = 0;
    }

    m_Fence->Wait(m_FrameFenceValues[m_CurrentFrame]);
    AcquireImage();
}

VulkanSwapchain::~VulkanSwapchain() {
    vkDeviceWaitIdle(m_Device->GetVkDevice());
    if (m_Fence) {
        delete m_Fence;
    }
    DestroySync();
    DestroySwapchain();
    DestroySurface();
    DestroyWindow();
}

Texture* VulkanSwapchain::GetCurrentBackBuffer() {
    return m_Textures[m_ImageIndex];
}

void VulkanSwapchain::UpdateWindow() {
    glfwPollEvents();
}

bool VulkanSwapchain::WindowShouldClose() {
    return glfwWindowShouldClose(m_Window);
}

void VulkanSwapchain::Present() {
    m_FrameFenceValues[m_CurrentFrame] = ++m_FenceValue;

    m_Queue->AddSignalSemaphore(m_RenderSemaphores[m_ImageIndex]);
    m_Queue->Signal(m_Fence, m_FenceValue);
    m_Queue->Flush();

    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &m_RenderSemaphores[m_ImageIndex],
        .swapchainCount = 1,
        .pSwapchains = &m_SwapChain,
        .pImageIndices = &m_ImageIndex
    };
    VkResult res = vkQueuePresentKHR(m_Queue->GetVkQueue(), &presentInfo);

    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        vkDeviceWaitIdle(m_Device->GetVkDevice());
        DestroySync();
        DestroySwapchain();
        CreateSwapchain();
        CreateSync();
    }

    m_CurrentFrame = (m_CurrentFrame + 1) % 3;
    m_Fence->Wait(m_FrameFenceValues[m_CurrentFrame]);
    AcquireImage();
}

void VulkanSwapchain::AcquireImage() {
    VkResult res = vkAcquireNextImageKHR(m_Device->GetVkDevice(), m_SwapChain,
        UINT64_MAX, m_AcquireSemaphores[m_CurrentFrame], nullptr, &m_ImageIndex);

    if (res == VK_ERROR_OUT_OF_DATE_KHR) {
        vkDeviceWaitIdle(m_Device->GetVkDevice());
        DestroySync();
        DestroySwapchain();
        CreateSwapchain();
        CreateSync();
        AcquireImage();
    }

    m_Queue->AddWaitSemaphore(m_AcquireSemaphores[m_CurrentFrame]);
}

void VulkanSwapchain::CreateWindow() {

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_Window = glfwCreateWindow(800, 600, "RHI", nullptr, nullptr);
}

void VulkanSwapchain::CreateSurface() {
    VkResult res = glfwCreateWindowSurface(m_Device->GetVkInstance(), m_Window, nullptr, &m_Surface);
}

void VulkanSwapchain::CheckQueueSupport() {
    VkBool32 support;
    vkGetPhysicalDeviceSurfaceSupportKHR(m_Device->GetVkPhysicalDevice(),
        m_Queue->GetQueueFamilyIndex(), m_Surface, &support);
}

void VulkanSwapchain::CreateSwapchain() {
    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_Device->GetVkPhysicalDevice(), m_Surface, &surfaceCapabilities);
    uint32_t surfaceFormatCount;
    std::vector<VkSurfaceFormatKHR> surfaceFormats;
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_Device->GetVkPhysicalDevice(), m_Surface, &surfaceFormatCount, nullptr);
    surfaceFormats.resize(surfaceFormatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(m_Device->GetVkPhysicalDevice(), m_Surface, &surfaceFormatCount, surfaceFormats.data());

    VkSwapchainCreateInfoKHR swapchainCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = m_Surface,
        .minImageCount = surfaceCapabilities.minImageCount,
        .imageFormat = surfaceFormats[0].format,
        .imageColorSpace = surfaceFormats[0].colorSpace,
        .imageExtent = surfaceCapabilities.currentExtent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .preTransform = surfaceCapabilities.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR,
        .clipped = true
    };

    VkResult res = vkCreateSwapchainKHR(m_Device->GetVkDevice(), &swapchainCreateInfo, nullptr, &m_SwapChain);

    uint32_t imageCount;
    std::vector<VkImage> swapchainImages;
    vkGetSwapchainImagesKHR(m_Device->GetVkDevice(), m_SwapChain, &imageCount, nullptr);
    swapchainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(m_Device->GetVkDevice(), m_SwapChain, &imageCount, swapchainImages.data());

    m_Textures.resize(imageCount);
    for (int i = 0; i < imageCount; i++) {
        TextureDesc desc = {
            .Width = surfaceCapabilities.currentExtent.width,
            .Height = surfaceCapabilities.currentExtent.height,
            .MipLevels = 1,
            .ArrayLayers = 1,
            .Samples = 1,
            .Format = TextureFormat::B8G8R8A8_UNORM,
            .Type = TextureType::Texture2D
        };
        m_Textures[i] = new VulkanTexture(desc, m_Device, swapchainImages[i]);
    }
}

void VulkanSwapchain::CreateSync() {
    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
    };
    m_AcquireSemaphores.resize(3);
    for (size_t i = 0; i < 3; i++) {
        VkResult res = vkCreateSemaphore(m_Device->GetVkDevice(), &semaphoreCreateInfo, nullptr, &m_AcquireSemaphores[i]);
    }
    m_RenderSemaphores.resize(m_Textures.size());
    for (size_t i = 0; i < m_RenderSemaphores.size(); i++) {
        VkResult res = vkCreateSemaphore(m_Device->GetVkDevice(), &semaphoreCreateInfo, nullptr, &m_RenderSemaphores[i]);
    }
}

void VulkanSwapchain::DestroySync() {
    for (size_t i = 0; i < 3; i++) {
        vkDestroySemaphore(m_Device->GetVkDevice(), m_AcquireSemaphores[i], nullptr);
    }
    for (size_t i = 0; i < m_RenderSemaphores.size(); i++) {
        vkDestroySemaphore(m_Device->GetVkDevice(), m_RenderSemaphores[i], nullptr);
    }
}

void VulkanSwapchain::DestroySwapchain() {
    for (auto texture : m_Textures) {
        delete texture;
    }
    if (m_SwapChain) {
        vkDestroySwapchainKHR(m_Device->GetVkDevice(), m_SwapChain, nullptr);
    }
}

void VulkanSwapchain::DestroySurface() {
    if (m_Surface) {
        vkDestroySurfaceKHR(m_Device->GetVkInstance(), m_Surface, nullptr);
    }
}

void VulkanSwapchain::DestroyWindow() {
    glfwDestroyWindow(m_Window);
}

} // vk