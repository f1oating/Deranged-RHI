//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_VULKANCOMMANDQUEUE_H
#define DERANGED_RHI_VULKANCOMMANDQUEUE_H

#include <volk.h>
#include "CommandQueue.h"

class VulkanDevice;

class VulkanCommandQueue : public CommandQueue {
public:
    VulkanCommandQueue(uint32_t queueIndex, VulkanDevice* device);
    ~VulkanCommandQueue() override;

    uint32_t GetQueueFamilyIndex() const { return m_QueueIndex; };
    VkQueue GetVkQueue() const { return m_Queue; };

private:
    uint32_t m_QueueIndex = 0;
    VulkanDevice* m_Device;
    VkQueue m_Queue = nullptr;

};

#endif //DERANGED_RHI_VULKANCOMMANDQUEUE_H
