//
// Created by alan on 13/08/2026.
//

#include "Backend/Vulkan/Internal/RingBuffer.h"

namespace vk {

void RingBuffer::Init(VkDevice device, VkPhysicalDevice physDevice, uint64_t size) {
    m_Device = device;
    m_PhysDevice = physDevice;
    m_Size = size;

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_PhysDevice, &memoryProperties);

    uint32_t memoryTypeIndex = 0;
    for (int i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            memoryTypeIndex = i;
            break;
        }
    }

    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = m_Size,
        .memoryTypeIndex = memoryTypeIndex,
    };

    VkResult res = vkAllocateMemory(m_Device, &memoryAllocateInfo, nullptr, &m_Memory);
    vkMapMemory(m_Device, m_Memory, 0, m_Size, 0, &m_Mapped);
}

void RingBuffer::Shutdown() {
    if (m_Memory) {
        vkUnmapMemory(m_Device, m_Memory);
        vkFreeMemory(m_Device, m_Memory, nullptr);
    }
}

uint64_t RingBuffer::Allocate(uint64_t size) {
    uint64_t alignedSize = AlignUp(size, (uint64_t)256);

    if (m_Head + alignedSize > m_Size) {
        m_Head = 0;
    }

    uint64_t offset = m_Head;
    m_Head += alignedSize;

    return offset;
}

} // vk
