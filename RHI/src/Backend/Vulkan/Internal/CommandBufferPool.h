//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_COMMANDBUFFERPOOL_H
#define DERANGED_RHI_COMMANDBUFFERPOOL_H

#include <deque>
#include <volk.h>

namespace vk {

class CommandBufferPool {
public:
    CommandBufferPool(VkDevice device, uint32_t queueFamily);
    ~CommandBufferPool();
    CommandBufferPool(const CommandBufferPool& other) = delete;
    CommandBufferPool& operator=(const CommandBufferPool& other) = delete;
    CommandBufferPool(CommandBufferPool&& other) = delete;
    CommandBufferPool& operator=(CommandBufferPool&& other) = delete;

    VkCommandBuffer AcquireCommandBuffer();
    void ReleaseCommandBuffer(VkCommandBuffer commandBuffer, uint64_t value);

    void Poll(uint64_t value);

private:
    VkDevice m_Device = nullptr;
    VkCommandPool m_CommandPool = nullptr;
    std::deque<VkCommandBuffer> m_AcquireBuffers;
    std::deque<std::pair<VkCommandBuffer, uint64_t>> m_ReleaseBuffers;

};

} // vk

#endif //DERANGED_RHI_COMMANDBUFFERPOOL_H
