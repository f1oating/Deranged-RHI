//
// Created by alan on 08/09/2026.
//

#include "Backend/Vulkan/Internal/DescriptorPool.h"

namespace vk {

DescriptorPool::DescriptorPool(VkDevice device) {
    m_Device = device;

    VkDescriptorPoolSize sizes[] = {
        { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 8 },
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 8 }
    };

    VkDescriptorPoolCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
        .maxSets = m_Size,
        .poolSizeCount = sizeof(sizes) / sizeof(sizes[0]),
        .pPoolSizes = sizes
    };
    vkCreateDescriptorPool(m_Device, &createInfo, nullptr, &m_DescriptorPool);
}

DescriptorPool::~DescriptorPool() {
    if (m_DescriptorPool) {
        vkDestroyDescriptorPool(m_Device, m_DescriptorPool, nullptr);
    }
}

VkDescriptorSet DescriptorPool::Allocate(VkDescriptorSetLayout layout) {
    VkDescriptorSet descriptorSet = nullptr;

    VkDescriptorSetAllocateInfo allocInfo = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = m_DescriptorPool,
        .descriptorSetCount = 1,
        .pSetLayouts = &layout,
    };
    vkAllocateDescriptorSets(m_Device, &allocInfo, &descriptorSet);

    --m_Size;
    return descriptorSet;
}

void DescriptorPool::Free(VkDescriptorSet set) {
    vkFreeDescriptorSets(m_Device, m_DescriptorPool, 1, &set);
    ++m_Size;
}

DescriptorManager::DescriptorManager(VkDevice device) {
    m_Device = device;
    m_DescriptorPool = std::make_unique<DescriptorPool>(m_Device);
}

DescriptorManager::~DescriptorManager() {
    m_DescriptorPool.reset();
}

void DescriptorManager::SetDescriptorState(DescriptorState descriptorState) {
    m_DescriptorState = descriptorState;
}

void DescriptorManager::WriteBufferInfo(uint32_t set, uint32_t binding, VkDescriptorBufferInfo bufferInfo) {
    m_DescriptorState.Sets[set].Descriptors[binding].BufferInfo = bufferInfo;
}

void DescriptorManager::WriteImageInfo(uint32_t set, uint32_t binding, VkDescriptorImageInfo imageInfo) {
    m_DescriptorState.Sets[set].Descriptors[binding].ImageInfo = imageInfo;
}

void DescriptorManager::WriteAndBind(VkCommandBuffer commandBuffer, VkPipelineLayout layout, uint64_t frame) {
    std::vector<VkDescriptorSet> descriptorSets;
    for (int i = 0; i < 8; i++) {
        if (m_DescriptorState.SetIndices & (1 << i)) {
            m_DescriptorState.Sets[i].Set = m_DescriptorPool->Allocate(m_DescriptorState.Sets[i].Layout);
            descriptorSets.push_back(m_DescriptorState.Sets[i].Set);
        }
    }

    std::vector<VkWriteDescriptorSet> writes;

    for (int i = 0; i < 8; i++) {
        if (!(m_DescriptorState.SetIndices & (1 << i))) continue;

        for (int j = 0; j < 32; j++) {
            if (!(m_DescriptorState.Sets[i].DescriptorIndices & (1 << j))) continue;

            VkWriteDescriptorSet writeDescriptorSet{};
            writeDescriptorSet.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            writeDescriptorSet.descriptorCount = 1;
            writeDescriptorSet.dstBinding = j;
            writeDescriptorSet.dstSet = m_DescriptorState.Sets[i].Set;
            writeDescriptorSet.dstArrayElement = 0;
            writeDescriptorSet.descriptorType = m_DescriptorState.Sets[i].Descriptors[j].Type;

            if (m_DescriptorState.Sets[i].Descriptors[j].Type == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER) {
                writeDescriptorSet.pBufferInfo = &m_DescriptorState.Sets[i].Descriptors[j].BufferInfo;
            }

            if (m_DescriptorState.Sets[i].Descriptors[j].Type == VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE) {
                writeDescriptorSet.pImageInfo = &m_DescriptorState.Sets[i].Descriptors[j].ImageInfo;
            }

            if (m_DescriptorState.Sets[i].Descriptors[j].Type == VK_DESCRIPTOR_TYPE_SAMPLER) {
                writeDescriptorSet.pImageInfo = &m_DescriptorState.Sets[i].Descriptors[j].ImageInfo;
            }

            writes.push_back(writeDescriptorSet);
        }
    }

    vkUpdateDescriptorSets(m_Device, writes.size(), writes.data(), 0, nullptr);

    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0,
        descriptorSets.size(), descriptorSets.data(), 0, nullptr);

    for (auto descriptorSet : descriptorSets) {
        m_ReleaseQueue.emplace_back(descriptorSet, frame);
    }
}

void DescriptorManager::Free(uint64_t frame) {
    while (!m_ReleaseQueue.empty()) {
        const auto& [set, value] = m_ReleaseQueue.front();
        if (value <= frame) {
            m_DescriptorPool->Free(set);
            m_ReleaseQueue.pop_front();
            continue;
        }
        break;
    }
}

} // vk