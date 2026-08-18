//
// Created by alan on 09/08/2026.
//

#ifndef DERANGED_RHI_VULKANFENCE_H
#define DERANGED_RHI_VULKANFENCE_H

#include "Fence.h"
#include <volk.h>

#include "ReleaseManager.h"

namespace vk {

class VulkanDevice;

class VulkanFence : public Fence {
public:
    VulkanFence(VulkanDevice* device);
    ~VulkanFence() override;

    uint64_t GetCompletedValue() override;

    void Wait(uint64_t value) override;

    VkSemaphore GetVkSemaphore() const { return m_TimelineSemaphore; }

private:
    VulkanDevice* m_Device = nullptr;
    VkSemaphore m_TimelineSemaphore = nullptr;

};

struct FenceReleaseResource : ReleaseResourceBase {
    VkDevice Device;
    VkSemaphore TimelineSemaphore;

    FenceReleaseResource(VkDevice device, VkSemaphore timelineSemaphore)
        : Device(device), TimelineSemaphore(timelineSemaphore) {}

    void Destroy() override {
        vkDestroySemaphore(Device, TimelineSemaphore, nullptr);
    }

};

} // vk

#endif //DERANGED_RHI_VULKANFENCE_H
