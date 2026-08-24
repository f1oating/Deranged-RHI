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
        VK_DYNAMIC_STATE_SCISSOR
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
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = false
    };

    VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = false,
        .rasterizerDiscardEnable = false,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .cullMode = VK_CULL_MODE_NONE,
        .frontFace = VK_FRONT_FACE_CLOCKWISE,
        .depthBiasEnable = false,
        .depthBiasConstantFactor = 0.0f,
        .depthBiasClamp = 0.0f,
        .depthBiasSlopeFactor = 0.0f,
        .lineWidth = 1.0f,
    };

    VkPipelineTessellationStateCreateInfo tessellationStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO
    };

    VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = false,
        .depthWriteEnable = false,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = false,
        .stencilTestEnable = false,
        .minDepthBounds = 0.0f,
        .maxDepthBounds = 1.0f,
    };

    VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {
        .blendEnable = false,
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT
            | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
    };

    VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = false,
        .logicOp = VK_LOGIC_OP_CLEAR,
        .attachmentCount = 1,
        .pAttachments = &colorBlendAttachmentState
    };

    VkFormat colorAttachmentFormat = VK_FORMAT_B8G8R8A8_UNORM;

    VkPipelineRenderingCreateInfo renderingCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &colorAttachmentFormat
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