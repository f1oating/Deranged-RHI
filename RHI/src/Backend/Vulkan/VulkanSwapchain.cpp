//
// Created by alan on 08/08/2026.
//

#include "Backend/Vulkan/VulkanSwapchain.h"
#include "Backend/Vulkan/VulkanDevice.h"
#include <vector>

VulkanSwapchain::VulkanSwapchain(VulkanCommandQueue* queue, VulkanDevice* device) {
    m_Queue = queue;
    m_Device = device;
    CreateWindow();
    CreateSurface();
    CheckQueueSupport();
    CreateSwapchain();
}

VulkanSwapchain::~VulkanSwapchain() {
    DestroySwapchain();
    DestroySurface();
    DestroyWindow();
}

void VulkanSwapchain::UpdateWindow() {
    glfwPollEvents();
}

bool VulkanSwapchain::WindowShouldClose() {
    return glfwWindowShouldClose(m_Window);
}

void VulkanSwapchain::Present() {

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
}

void VulkanSwapchain::DestroySwapchain() {
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
