//
// Created by alan on 08/08/2026.
//

#include "Backend/Vulkan/VulkanCommandQueue.h"
#include "Backend/Vulkan/VulkanDevice.h"

VulkanCommandQueue::VulkanCommandQueue(uint32_t queueIndex, VulkanDevice* device) {
    m_QueueIndex = queueIndex;
    m_Device = device;
    vkGetDeviceQueue(m_Device->GetVkDevice(), m_QueueIndex, 0, &m_Queue);
}

VulkanCommandQueue::~VulkanCommandQueue() {

}