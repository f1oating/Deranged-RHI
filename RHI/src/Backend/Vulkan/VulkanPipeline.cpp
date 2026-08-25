//
// Created by alan on 11/08/2026.
//

#include "Backend/Vulkan/VulkanPipeline.h"
#include "Backend/Vulkan/VulkanDevice.h"
#include <vector>

namespace vk {

VulkanGraphicsPipelineState::VulkanGraphicsPipelineState(GraphicsPipelineDesc desc, VulkanDevice* device) {
    m_Device = device;
    m_Desc = desc;

    CreatePipelineLayout();
    CreatePipeline();
}

VulkanGraphicsPipelineState::~VulkanGraphicsPipelineState() {
    if (m_Pipeline && m_Layout) {
        m_Device->ReleaseResource(
            new PipelineStateReleaseResource(m_Device->GetVkDevice(), m_Layout,m_Pipeline));
    }
}

GraphicsPipelineDesc VulkanGraphicsPipelineState::GetDesc() {
    return m_Desc;
}

void VulkanGraphicsPipelineState::CreatePipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
    };
    vkCreatePipelineLayout(m_Device->GetVkDevice(), &pipelineLayoutCreateInfo, nullptr, &m_Layout);
}

void VulkanGraphicsPipelineState::CreatePipeline() {
    VkShaderModule vertexShader = nullptr;
    VkShaderModule fragmentShader = nullptr;

    std::vector<VkPipelineShaderStageCreateInfo> shaderStages;

    if (m_Desc.VertexShader.Data) {
        VkShaderModuleCreateInfo shaderModuleCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = m_Desc.VertexShader.Size,
            .pCode = (uint32_t*)m_Desc.VertexShader.Data,
        };
        vkCreateShaderModule(m_Device->GetVkDevice(), &shaderModuleCreateInfo, nullptr, &vertexShader);

        VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vertexShader,
            .pName = "main"
        };
        shaderStages.push_back(shaderStageCreateInfo);
    }

    if (m_Desc.FragmentShader.Data) {
        VkShaderModuleCreateInfo shaderModuleCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = m_Desc.FragmentShader.Size,
            .pCode = (uint32_t*)m_Desc.FragmentShader.Data,
        };
        vkCreateShaderModule(m_Device->GetVkDevice(), &shaderModuleCreateInfo, nullptr, &fragmentShader);

        VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = fragmentShader,
            .pName = "main"
        };
        shaderStages.push_back(shaderStageCreateInfo);
    }

    std::vector<VkDynamicState> dynamicState = {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        VK_DYNAMIC_STATE_BLEND_CONSTANTS
    };

    VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = (uint32_t)dynamicState.size(),
        .pDynamicStates = dynamicState.data(),
    };

    std::vector<VkVertexInputAttributeDescription> inputAttributes;

    uint32_t offset = 0;
    for (int i = 0; i < m_Desc.VertexInput.InputElements.size(); i++) {
        VkVertexInputAttributeDescription attributeDescription = {
            .location = (uint32_t)i,
            .binding = 0,
            .format = ToVkFormat(m_Desc.VertexInput.InputElements[i].Type),
            .offset = offset
        };
        inputAttributes.push_back(attributeDescription);
        offset += ToSize(m_Desc.VertexInput.InputElements[i].Type);
    }

    VkVertexInputBindingDescription inputBinding = {
        .binding = 0,
        .stride = offset,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
    };

    VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = m_Desc.VertexInput.InputElements.size() ? 1U : 0U,
        .pVertexBindingDescriptions = m_Desc.VertexInput.InputElements.size() ? &inputBinding : nullptr,
        .vertexAttributeDescriptionCount = (uint32_t)inputAttributes.size(),
        .pVertexAttributeDescriptions = inputAttributes.data()
    };

    VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
        .sampleShadingEnable = false,
        .pSampleMask = nullptr,
        .alphaToCoverageEnable = false,
        .alphaToOneEnable = false,
    };

    VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .scissorCount = 1
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = ToVkPrimitiveTopology(m_Desc.PrimitiveTopology),
        .primitiveRestartEnable = false
    };

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = m_Desc.Rasterization.DepthBiasClamp > 0.0f,
        .rasterizerDiscardEnable = false,
        .polygonMode = ToVkPolygonMode(m_Desc.Rasterization.Polygon),
        .cullMode = ToVkCullMode(m_Desc.Rasterization.Cull),
        .frontFace = ToVkFrontFace(m_Desc.Rasterization.Face),
        .depthBiasEnable = m_Desc.Rasterization.DepthBiasConstant > 0,
        .depthBiasConstantFactor = m_Desc.Rasterization.DepthBiasConstant,
        .depthBiasClamp = m_Desc.Rasterization.DepthBiasClamp,
        .depthBiasSlopeFactor = m_Desc.Rasterization.DepthBiasSlope,
        .lineWidth = 1.0f,
    };

    VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO
    };

    VkStencilOpState frontState = {
        .failOp = ToVkStencilOp(m_Desc.DepthStencil.Front.Fail),
        .passOp = ToVkStencilOp(m_Desc.DepthStencil.Front.Pass),
        .depthFailOp = ToVkStencilOp(m_Desc.DepthStencil.Front.DepthFail),
        .compareOp = ToVkCompareOp(m_Desc.DepthStencil.Front.StencilFunc),
        .compareMask = m_Desc.DepthStencil.StencilReadMask,
        .writeMask = m_Desc.DepthStencil.StencilWriteMask
    };

    VkStencilOpState backState = {
        .failOp = ToVkStencilOp(m_Desc.DepthStencil.Back.Fail),
        .passOp = ToVkStencilOp(m_Desc.DepthStencil.Back.Pass),
        .depthFailOp = ToVkStencilOp(m_Desc.DepthStencil.Back.DepthFail),
        .compareOp = ToVkCompareOp(m_Desc.DepthStencil.Back.StencilFunc),
        .compareMask = m_Desc.DepthStencil.StencilReadMask,
        .writeMask = m_Desc.DepthStencil.StencilWriteMask
    };

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = m_Desc.DepthStencil.DepthEnable,
        .depthWriteEnable = m_Desc.DepthStencil.DepthWriteEnable,
        .depthCompareOp = ToVkCompareOp(m_Desc.DepthStencil.DepthCompare),
        .depthBoundsTestEnable = false,
        .stencilTestEnable = m_Desc.DepthStencil.StencilEnable,
        .front = frontState,
        .back = backState,
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f
    };

    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachments;
    for (auto attachment : m_Desc.Blend.ColorAttachments) {
        VkPipelineColorBlendAttachmentState colorBlendAttachment = {
            .blendEnable = attachment.BlendEnable,
            .srcColorBlendFactor = ToVkBlendFactor(attachment.SrcColorBlend),
            .dstColorBlendFactor = ToVkBlendFactor(attachment.DstColorBlend),
            .colorBlendOp = ToVkBlendOp(attachment.ColorBlend),
            .srcAlphaBlendFactor = ToVkBlendFactor(attachment.SrcAlphaBlend),
            .dstAlphaBlendFactor = ToVkBlendFactor(attachment.DstAlphaBlend),
            .alphaBlendOp = ToVkBlendOp(attachment.AlphaBlend),
            .colorWriteMask = ToVkColorWriteMask(attachment.ColorWriteMask)
        };
        colorBlendAttachments.push_back(colorBlendAttachment);
    }

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = m_Desc.Blend.LogicOpEnable,
        .logicOp = ToVkLogicOp(m_Desc.Blend.Logic),
        .attachmentCount = (uint32_t)colorBlendAttachments.size(),
        .pAttachments = colorBlendAttachments.data()
    };

    std::vector<VkFormat> colorAttachmentFormats;
    for (auto format : m_Desc.ColorFormats) {
        colorAttachmentFormats.push_back(ToVkFormat(format));
    }

    VkPipelineRenderingCreateInfo renderingCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = (uint32_t)colorAttachmentFormats.size(),
        .pColorAttachmentFormats = colorAttachmentFormats.data(),
        .depthAttachmentFormat = ToVkFormat(m_Desc.DepthStencilFormat),
        .stencilAttachmentFormat = HasStencilAspect(m_Desc.DepthStencilFormat) ? ToVkFormat(m_Desc.DepthStencilFormat) : VK_FORMAT_UNDEFINED
    };

    VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderingCreateInfo,
        .stageCount = (uint32_t)shaderStages.size(),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertexInputStateCreateInfo,
        .pInputAssemblyState = &inputAssemblyStateCreateInfo,
        .pTessellationState = &tessellationStateCreateInfo,
        .pViewportState = &viewportStateCreateInfo,
        .pRasterizationState = &rasterizationStateCreateInfo,
        .pMultisampleState = &multisampleStateCreateInfo,
        .pDepthStencilState = &depthStencilStateCreateInfo,
        .pColorBlendState = &colorBlendStateCreateInfo,
        .pDynamicState = &dynamicStateCreateInfo,
        .layout = m_Layout,
    };

    vkCreateGraphicsPipelines(m_Device->GetVkDevice(), nullptr,
        1, &graphicsPipelineCreateInfo, nullptr, &m_Pipeline);

    if (vertexShader) {
        vkDestroyShaderModule(m_Device->GetVkDevice(), vertexShader, nullptr);
    }
    if (fragmentShader) {
        vkDestroyShaderModule(m_Device->GetVkDevice(), fragmentShader, nullptr);
    }
}

} // vk