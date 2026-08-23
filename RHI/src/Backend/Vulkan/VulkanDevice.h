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
    Buffer* CreateBuffer(BufferDesc desc) override;

    void ReleaseResource(ReleaseResourceBase* resource);

    uint32_t FindMemoryTypeIndex(uint32_t memoryTypeBits, uint32_t propertyFlags);

    VkInstance GetVkInstance() { return m_Instance; }
    VkDevice GetVkDevice() const { return m_Device; }
    VkPhysicalDevice GetVkPhysicalDevice() const { return m_PhysicalDevice; }
    RingBuffer* GetRingBuffer() { return &m_RingBuffer; }

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
    RingBuffer m_RingBuffer;

};

struct RingBufferReleaseResource : ReleaseResourceBase {
    RingBuffer* Buffer;
    uint64_t Tail;

    RingBufferReleaseResource(RingBuffer* buffer, uint64_t tail)
        : Buffer(buffer), Tail(tail) {}

    void Destroy() override {
        Buffer->SetTail(Tail);
    }

};

} // vk

#endif //DERANGED_RHI_VULKANDEVICE_H
