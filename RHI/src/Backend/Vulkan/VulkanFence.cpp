//
// Created by alan on 09/08/2026.
//

#include "Backend/Vulkan/VulkanFence.h"

#include "VulkanDevice.h"

VulkanFence::VulkanFence(VulkanDevice* device) {
    m_Device = device;

    VkSemaphoreTypeCreateInfo typeCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = 0
    };
    VkSemaphoreCreateInfo semaphoreCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &typeCreateInfo
    };

    VkResult res = vkCreateSemaphore(m_Device->GetVkDevice(), &semaphoreCreateInfo, nullptr, &m_TimelineSemaphore);
}

uint64_t VulkanFence::GetCompletedValue() {
    uint64_t value;
    vkGetSemaphoreCounterValue(m_Device->GetVkDevice(), m_TimelineSemaphore, &value);
    return value;
}

void VulkanFence::Wait(uint64_t value) {
    VkSemaphoreWaitInfo semaphoreWaitInfo = {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .semaphoreCount = 1,
        .pSemaphores = &m_TimelineSemaphore,
        .pValues = &value
    };
    vkWaitSemaphores(m_Device->GetVkDevice(), &semaphoreWaitInfo,UINT64_MAX);
}

VulkanFence::~VulkanFence() {
    if (m_TimelineSemaphore) {
        vkDestroySemaphore(m_Device->GetVkDevice(), m_TimelineSemaphore, nullptr);
    }
}