//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_VULKANDEVICE_H
#define DERANGED_RHI_VULKANDEVICE_H

#include <volk.h>
#include <optional>
#include "Device.h"
#include "Backend/Vulkan/VulkanCommandQueue.h"
#include "Backend/Vulkan/Internal/RingBuffer.h"

namespace vk {

class VulkanDevice : public Device {
public:
    VulkanDevice();
    ~VulkanDevice() override;

    void EndFrame() override;

    CommandQueue* GetCommandQueue() override;
    Swapchain* CreateSwapchain() override;
    GraphicsPipelineState* CreateGraphicsPipelineState(GraphicsPipelineDesc desc) override;
    Texture* CreateTexture(TextureDesc desc) override;
    TextureView* CreateTextureView(TextureViewDesc desc) override;

    void ReleaseResource(ReleaseResourceBase* resource);

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
    vk::RingBuffer m_RingBuffer;

};

} // vk

#endif //DERANGED_RHI_VULKANDEVICE_H
