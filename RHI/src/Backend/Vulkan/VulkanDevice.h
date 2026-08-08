//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_VULKANDEVICE_H
#define DERANGED_RHI_VULKANDEVICE_H

#include <volk.h>
#include <optional>
#include "Device.h"
#include "Backend/Vulkan/VulkanCommandQueue.h"

class VulkanDevice : public Device {
public:
    VulkanDevice();
    ~VulkanDevice() override;

    Swapchain* CreateSwapchain() override;

    VkInstance GetVkInstance() { return m_Instance; }
    VkDevice GetVkDevice() const { return m_Device; }
    VkPhysicalDevice GetVkPhysicalDevice() const { return m_PhysicalDevice; }

private:
    void CreateInstance();
    void PickPhysicalDevice();
    void FindQueueFamilyIndex();
    void CreateLogicalDevice();

    void DestroyInstance();
    void DestroyLogicalDevice();

private:
    VkInstance m_Instance = nullptr;
    VkPhysicalDevice m_PhysicalDevice = nullptr;
    std::optional<uint32_t> m_QueueFamily;
    VkDevice m_Device = nullptr;
    VulkanCommandQueue* m_Queue = nullptr;

};

#endif //DERANGED_RHI_VULKANDEVICE_H
