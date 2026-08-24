//
// Created by alan on 11/08/2026.
//

#ifndef DERANGED_RHI_VULKANPIPELINE_H
#define DERANGED_RHI_VULKANPIPELINE_H

#include "Pipeline.h"
#include <volk.h>
#include "ReleaseManager.h"

namespace vk {

class VulkanDevice;

class VulkanGraphicsPipelineState : public GraphicsPipelineState {
public:
    VulkanGraphicsPipelineState(GraphicsPipelineDesc desc, VulkanDevice* device);
    ~VulkanGraphicsPipelineState() override;

    VkPipeline GetPipeline() const { return m_Pipeline; }

private:
    void CreatePipelineLayout();
    void CreatePipeline();

private:
    VulkanDevice* m_Device = nullptr;
    VkPipelineLayout m_Layout = nullptr;
    VkPipeline m_Pipeline = nullptr;
    GraphicsPipelineDesc m_Desc;

};

struct PipelineStateReleaseResource : ReleaseResourceBase {
    VkDevice Device;
    VkPipelineLayout Layout;
    VkPipeline Pipeline;

    PipelineStateReleaseResource(VkDevice device, VkPipelineLayout layout, VkPipeline pipeline)
        : Device(device), Layout(layout), Pipeline(pipeline) {}

    void Destroy() override {
        vkDestroyPipelineLayout(Device, Layout, nullptr);
        vkDestroyPipeline(Device, Pipeline, nullptr);
    }

};

inline uint32_t ToSize(ValueType type) {
    switch (type) {
        case ValueType::Int:
        case ValueType::Float:
            return 4;
        case ValueType::Int2:
        case ValueType::Float2:
            return 8;
        case ValueType::Int3:
        case ValueType::Float3:
            return 12;
        case ValueType::Int4:
        case ValueType::Float4:
            return 16;
        default:
            return 4;
    }
}

inline VkFormat ToVkFormat(ValueType type) {
    switch (type) {
        case ValueType::Int:
            return VK_FORMAT_R32_SINT;
        case ValueType::Int2:
            return VK_FORMAT_R32G32_SINT;
        case ValueType::Int3:
            return VK_FORMAT_R32G32B32_SINT;
        case ValueType::Int4:
            return VK_FORMAT_R32G32B32A32_SINT;

        case ValueType::Float:
            return VK_FORMAT_R32_SFLOAT;
        case ValueType::Float2:
            return VK_FORMAT_R32G32_SFLOAT;
        case ValueType::Float3:
            return VK_FORMAT_R32G32B32_SFLOAT;
        case ValueType::Float4:
            return VK_FORMAT_R32G32B32A32_SFLOAT;

        default:
            return VK_FORMAT_R32_SINT;
    }
}

} // vk

#endif //DERANGED_RHI_VULKANPIPELINE_H
