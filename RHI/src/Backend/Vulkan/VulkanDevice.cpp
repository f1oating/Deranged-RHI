//
// Created by alan on 08/08/2026.
//

#include "Backend/Vulkan/VulkanDevice.h"
#include <vector>
#include "Backend/Vulkan/VulkanSwapchain.h"
#include <GLFW/glfw3.h>
#include "Backend/Vulkan/VulkanResource.h"
#include "Backend/Vulkan/VulkanPipeline.h"

namespace vk {

VulkanDevice::VulkanDevice() {
    glfwInit();
    CreateInstance();
    PickPhysicalDevice();
    FindQueueFamilyIndex();
    CreateLogicalDevice();
    m_RingBuffer.Init(m_Device, m_PhysicalDevice);
    m_Queue = new VulkanCommandQueue(m_QueueFamily.value(), this);
}

VulkanDevice::~VulkanDevice() {
    vkDeviceWaitIdle(m_Device);
    delete m_Queue;
    m_RingBuffer.Shutdown();
    DestroyLogicalDevice();
    DestroyInstance();
    glfwTerminate();
}

void VulkanDevice::EndFrame() {
    ReleaseResource(new RingBufferReleaseResource(&m_RingBuffer, m_RingBuffer.GetHead()));
    m_Queue->EndFrame();
}

CommandQueue* VulkanDevice::GetCommandQueue() {
    return m_Queue;
}

Swapchain* VulkanDevice::CreateSwapchain() {
    return new VulkanSwapchain(m_Queue, this);
}

GraphicsPipelineState* VulkanDevice::CreateGraphicsPipelineState(GraphicsPipelineDesc desc) {
    return new VulkanGraphicsPipelineState(desc, this);
}

Texture* VulkanDevice::CreateTexture(TextureDesc desc) {
    return new VulkanTexture(desc, this);
}

TextureView* VulkanDevice::CreateTextureView(TextureViewDesc desc) {
    return new VulkanTextureView(desc, this);
}

Buffer* VulkanDevice::CreateBuffer(BufferDesc desc) {
    return new VulkanBuffer(desc, this);
}

void VulkanDevice::ReleaseResource(ReleaseResourceBase *resource) {
    m_Queue->ReleaseResource(new ReleaseResourceWrapper(resource, 1));
}

uint32_t VulkanDevice::FindMemoryTypeIndex(uint32_t memoryTypeBits, uint32_t propertyFlags) {
    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_PhysicalDevice, &physicalDeviceMemoryProperties);

    uint32_t memoryTypeIndex = 0;
    for (int i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) {
        if (!(memoryTypeBits & ( 1U << i))) continue;

        if ((physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & propertyFlags) != propertyFlags) {
            continue;
        }

        memoryTypeIndex = i;
        break;
    }

    return memoryTypeIndex;
}

void VulkanDevice::CreateInstance() {
    volkInitialize();
    VkApplicationInfo appInfo = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = "RHI",
        .applicationVersion = VK_MAKE_API_VERSION(1, 1, 0, 0),
        .pEngineName = "Deranged",
        .engineVersion = VK_MAKE_API_VERSION(1, 1, 0, 0),
        .apiVersion = VK_API_VERSION_1_3
    };

    std::vector<const char*> layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    uint32_t extensionCount;
    const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

    VkInstanceCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .pApplicationInfo = &appInfo,
        .enabledLayerCount = (uint32_t)layers.size(),
        .ppEnabledLayerNames = layers.data(),
        .enabledExtensionCount = extensionCount,
        .ppEnabledExtensionNames = extensions
    };

    VkResult res = vkCreateInstance(&createInfo, nullptr, &m_Instance);
    volkLoadInstance(m_Instance);
}

void VulkanDevice::PickPhysicalDevice() {
    uint32_t deviceCount = 0;
    std::vector<VkPhysicalDevice> physicalDevices;
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, nullptr);
    physicalDevices.resize(deviceCount);
    vkEnumeratePhysicalDevices(m_Instance, &deviceCount, physicalDevices.data());

    for (uint32_t i = 0; i < deviceCount; i++) {
        VkPhysicalDeviceProperties props;
        vkGetPhysicalDeviceProperties(physicalDevices[i], &props);

        if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
            m_PhysicalDevice = physicalDevices[i];
            break;
        }
    }
}

void VulkanDevice::FindQueueFamilyIndex() {
    uint32_t familyCount = 0;
    std::vector<VkQueueFamilyProperties> queueFamilies;
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &familyCount, nullptr);
    queueFamilies.resize(familyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(m_PhysicalDevice, &familyCount, queueFamilies.data());

    for (int i = 0; i < queueFamilies.size(); i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            m_QueueFamily = i;
            break;
        }
    }
}

void VulkanDevice::CreateLogicalDevice() {
    float priority = 1.0f;
    VkDeviceQueueCreateInfo queueCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = m_QueueFamily.value(),
        .queueCount = 1,
        .pQueuePriorities = &priority
    };

    std::vector<const char*> extensions = {
        "VK_KHR_swapchain"
    };

    VkPhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DYNAMIC_RENDERING_FEATURES,
        .dynamicRendering = true
    };

    VkPhysicalDeviceTimelineSemaphoreFeatures timelineSemaphoreFeatures = {
        .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TIMELINE_SEMAPHORE_FEATURES,
        .pNext = &dynamicRenderingFeatures,
        .timelineSemaphore = true
    };

    VkDeviceCreateInfo deviceInfo = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = &timelineSemaphoreFeatures,
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueCreateInfo,
        .enabledLayerCount = 0,
        .ppEnabledLayerNames = nullptr,
        .enabledExtensionCount = (uint32_t)extensions.size(),
        .ppEnabledExtensionNames = extensions.data()
    };

    VkResult res = vkCreateDevice(m_PhysicalDevice, &deviceInfo, nullptr, &m_Device);
}

void VulkanDevice::DestroyInstance() {
    if (m_Instance) {
        vkDestroyInstance(m_Instance, nullptr);
    }
}

void VulkanDevice::DestroyLogicalDevice() {
    if (m_Instance) {
        vkDestroyDevice(m_Device, nullptr);
    }
}

} // vk