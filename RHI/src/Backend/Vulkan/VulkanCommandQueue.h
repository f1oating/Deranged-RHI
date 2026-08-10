//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_VULKANCOMMANDQUEUE_H
#define DERANGED_RHI_VULKANCOMMANDQUEUE_H

#include <volk.h>
#include "CommandQueue.h"
#include "Backend/Vulkan/Internal/CommandBufferPool.h"
#include <vector>
#include "Backend/Vulkan/VulkanFence.h"
#include "ReleaseManager.h"

class VulkanDevice;

class VulkanCommandQueue : public CommandQueue {
public:
    VulkanCommandQueue(uint32_t queueIndex, VulkanDevice* device);
    ~VulkanCommandQueue() override;

    void Wait(Fence* fence, uint64_t value) override;
    void Signal(Fence* fence, uint64_t value) override;

    void EndFrame() override;
    void Flush() override;

    void AddWaitSemaphore(VkSemaphore waitSemaphore, uint64_t value = 1);
    void AddSignalSemaphore(VkSemaphore signalSemaphore, uint64_t value = 1);

    void ReleaseResource(ReleaseResourceWrapper* releaseResourceWrapper);

    uint32_t GetQueueFamilyIndex() const { return m_QueueIndex; };
    VkQueue GetVkQueue() const { return m_Queue; };

private:
    void AcquireCommandBuffer();

private:
    uint32_t m_QueueIndex = 0;
    VulkanDevice* m_Device;
    VkQueue m_Queue = nullptr;
    CommandBufferPool m_CommandBufferPool;
    VkCommandBuffer m_CommandBuffer = nullptr;
    uint64_t m_CommandBufferNumber = 0;
    std::vector<VkSemaphore> m_WaitSemaphores;
    std::vector<uint64_t> m_WaitSemaphoresValues;
    std::vector<VkSemaphore> m_SignalSemaphores;
    std::vector<uint64_t> m_SignalSemaphoresValues;
    VulkanFence* m_Fence = nullptr;
    uint64_t m_FenceValue = 0;
    ReleaseManager m_ReleaseManager;

};

#endif //DERANGED_RHI_VULKANCOMMANDQUEUE_H
