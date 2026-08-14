//
// Created by alan on 13/08/2026.
//

#include "Backend/Vulkan/Internal/RingBuffer.h"

namespace vk {

void RingBuffer::Init(VkDevice device, VkPhysicalDevice physDevice) {
    m_Device = device;
    m_PhysDevice = physDevice;

    VkBufferCreateInfo bufferInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = 2560,
        .usage = VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    vkCreateBuffer(m_Device, &bufferInfo, nullptr, &m_Buffer);

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(m_Device, m_Buffer, &memoryRequirements);

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_PhysDevice, &memoryProperties);

    uint32_t memoryTypeIndex = 0;
    for (int i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) {
            memoryTypeIndex = i;
            break;
        }
    }

    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = memoryTypeIndex,
    };

    vkAllocateMemory(m_Device, &memoryAllocateInfo, nullptr, &m_Memory);
}

void RingBuffer::Shutdown() {
    if (m_Buffer) {
        vkDestroyBuffer(m_Device, m_Buffer, nullptr);
    }
    if (m_Memory) {
        vkFreeMemory(m_Device, m_Memory, nullptr);
    }
}

uint64_t RingBuffer::Allocate(uint64_t size) {
    uint64_t alignedSize = AlignUp(size, (uint64_t)256);

    if (m_Head + alignedSize > 2560) {
        m_Head = 0;
    }

    uint64_t offset = m_Head;
    m_Head += alignedSize;

    return offset;
}

}
