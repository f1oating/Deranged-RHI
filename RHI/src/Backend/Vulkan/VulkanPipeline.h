//
// Created by alan on 11/08/2026.
//

#ifndef DERANGED_RHI_VULKANPIPELINE_H
#define DERANGED_RHI_VULKANPIPELINE_H

#include "Pipeline.h"
#include <volk.h>
#include "ReleaseManager.h"

class VulkanDevice;

class VulkanGraphicsPipelineState : public GraphicsPipelineState {
public:
    VulkanGraphicsPipelineState(GraphicsPipelineDesc desc, VulkanDevice* device);
    ~VulkanGraphicsPipelineState() override;

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

#endif //DERANGED_RHI_VULKANPIPELINE_H
