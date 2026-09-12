//
// Created by alan on 08/09/2026.
//

#ifndef DERANGED_RHI_DESCRIPTORPOOL_H
#define DERANGED_RHI_DESCRIPTORPOOL_H

#include <deque>
#include <vector>
#include <volk.h>

namespace vk {

class DescriptorPool {
public:
    void Init(VkDevice device);
    void Shutdown();

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
    void Init(VkDevice device);
    void Shutdown();

    void SetDescriptorState(std::vector<DescriptorSet> descriptorState);

    void WriteBufferInfo(uint32_t set, uint32_t binding, VkDescriptorBufferInfo bufferInfo);
    void WriteImageInfo(uint32_t set, uint32_t binding, VkDescriptorImageInfo imageInfo);

    void WriteAndBind(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint64_t frame);

    void Free(uint64_t frame);

private:
    VkDevice m_Device = nullptr;
    DescriptorPool m_DescriptorPool;
    std::vector<DescriptorSet> m_DescriptorState;
    std::deque<std::pair<VkDescriptorSet, uint64_t>> m_ReleaseQueue;

};

} // vk

#endif //DERANGED_RHI_DESCRIPTORPOOL_H
