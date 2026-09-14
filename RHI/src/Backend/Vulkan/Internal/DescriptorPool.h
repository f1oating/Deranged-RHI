//
// Created by alan on 08/09/2026.
//

#ifndef DERANGED_RHI_DESCRIPTORPOOL_H
#define DERANGED_RHI_DESCRIPTORPOOL_H

#include <deque>
#include <vector>
#include <volk.h>
#include <memory>

namespace vk {

class DescriptorPool {
public:
    DescriptorPool(VkDevice device);
    ~DescriptorPool();
    DescriptorPool(const DescriptorPool& other) = delete;
    DescriptorPool& operator=(const DescriptorPool& other) = delete;
    DescriptorPool(DescriptorPool&& other) = delete;
    DescriptorPool& operator=(DescriptorPool&& other) = delete;

    VkDescriptorSet Allocate(VkDescriptorSetLayout layout);
    void Free(VkDescriptorSet set);

private:
    VkDevice m_Device = nullptr;
    VkDescriptorPool m_DescriptorPool = nullptr;
    uint32_t m_Size = 128;

};

struct Descriptor {
    VkDescriptorType Type;
    union {
        VkDescriptorImageInfo ImageInfo;
        VkDescriptorBufferInfo BufferInfo;
    };
    uint32_t Binding;
};

struct DescriptorSet {
    VkDescriptorSetLayout Layout;
    std::vector<Descriptor> Descriptors;
};

class DescriptorManager {
public:
    DescriptorManager(VkDevice device);
    ~DescriptorManager();
    DescriptorManager(const DescriptorManager& other) = delete;
    DescriptorManager& operator=(const DescriptorManager& other) = delete;
    DescriptorManager(DescriptorManager&& other) = delete;
    DescriptorManager& operator=(DescriptorManager&& other) = delete;

    void SetDescriptorState(std::vector<DescriptorSet> descriptorState);

    void WriteBufferInfo(uint32_t set, uint32_t binding, VkDescriptorBufferInfo bufferInfo);
    void WriteImageInfo(uint32_t set, uint32_t binding, VkDescriptorImageInfo imageInfo);

    void WriteAndBind(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint64_t frame);

    void Free(uint64_t frame);

private:
    VkDevice m_Device = nullptr;
    std::unique_ptr<DescriptorPool> m_DescriptorPool = nullptr;
    std::vector<DescriptorSet> m_DescriptorState;
    std::deque<std::pair<VkDescriptorSet, uint64_t>> m_ReleaseQueue;

};

} // vk

#endif //DERANGED_RHI_DESCRIPTORPOOL_H
