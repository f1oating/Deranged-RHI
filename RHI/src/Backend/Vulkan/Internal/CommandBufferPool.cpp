//
// Created by alan on 08/08/2026.
//

#include "Backend/Vulkan/Internal/CommandBufferPool.h"

void CommandBufferPool::Init(VkDevice device, uint32_t queueFamily) {
    m_Device = device;

    VkCommandPoolCreateInfo commandPoolCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
        .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        .queueFamilyIndex = queueFamily
    };

    VkResult res = vkCreateCommandPool(m_Device, &commandPoolCreateInfo, nullptr, &m_CommandPool);
}

void CommandBufferPool::Shutdown() {
   if (m_CommandPool) {
       vkDestroyCommandPool(m_Device, m_CommandPool, nullptr);
   }
}

VkCommandBuffer CommandBufferPool::AcquireCommandBuffer() {
    if (!m_AcquireBuffers.empty()) {
        VkCommandBuffer commandBuffer = m_AcquireBuffers.front();
        m_AcquireBuffers.pop_front();
        return commandBuffer;
    }

    VkCommandBuffer commandBuffer;

    VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = m_CommandPool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };
    VkResult res = vkAllocateCommandBuffers(m_Device, &commandBufferAllocateInfo, &commandBuffer);

    return commandBuffer;
}

void CommandBufferPool::ReleaseCommandBuffer(VkCommandBuffer commandBuffer, uint64_t value) {
    m_ReleaseBuffers.emplace_back(commandBuffer, value);
}

void CommandBufferPool::Poll(uint64_t value) {
    while (!m_ReleaseBuffers.empty()) {
        if (m_ReleaseBuffers.front().second <= value) {
            m_AcquireBuffers.push_back(m_ReleaseBuffers.front().first);
            m_ReleaseBuffers.pop_front();
            continue;
        }
        break;
    }
}