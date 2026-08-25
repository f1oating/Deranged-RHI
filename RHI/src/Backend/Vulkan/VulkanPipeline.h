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

inline bool HasStencilAspect(TextureFormat format) {
    switch (format) {
        case TextureFormat::D24_UNORM_S8_UINT:
        case TextureFormat::D32_SFLOAT_S8_UINT:
            return true;
        default:
            return false;
    }
}

inline VkPolygonMode ToVkPolygonMode(PolygonMode mode) {
    switch (mode) {
        case PolygonMode::Fill:
            return VK_POLYGON_MODE_FILL;
        case PolygonMode::Line:
            return VK_POLYGON_MODE_LINE;
        default:
            return VK_POLYGON_MODE_FILL;
    }
}

inline VkCullModeFlags ToVkCullMode(CullMode mode) {
    switch (mode) {
        case CullMode::None:
            return VK_CULL_MODE_NONE;
        case CullMode::Front:
            return VK_CULL_MODE_FRONT_BIT;
        case CullMode::Back:
            return VK_CULL_MODE_BACK_BIT;
        default:
            return VK_CULL_MODE_NONE;
    }
}

inline VkFrontFace ToVkFrontFace(FrontFace face) {
    switch (face) {
        case FrontFace::CW:
            return VK_FRONT_FACE_CLOCKWISE;
        case FrontFace::CCW:
            return VK_FRONT_FACE_COUNTER_CLOCKWISE;
        default:
            return VK_FRONT_FACE_CLOCKWISE;
    }
}

inline VkBlendOp ToVkBlendOp(BlendOp op) {
    switch (op) {
        case BlendOp::Add:
            return VK_BLEND_OP_ADD;
        case BlendOp::Subtract:
            return VK_BLEND_OP_SUBTRACT;
        case BlendOp::ReverseSubtract:
            return VK_BLEND_OP_REVERSE_SUBTRACT;
        case BlendOp::Min:
            return VK_BLEND_OP_MIN;
        case BlendOp::Max:
            return VK_BLEND_OP_MAX;
        default:
            return VK_BLEND_OP_ADD;
    }
}

inline VkBlendFactor ToVkBlendFactor(BlendFactor factor) {
    switch (factor) {
        case BlendFactor::Zero:
            return VK_BLEND_FACTOR_ZERO;
        case BlendFactor::One:
            return VK_BLEND_FACTOR_ONE;
        case BlendFactor::SrcColor:
            return VK_BLEND_FACTOR_SRC_COLOR;
        case BlendFactor::InvSrcColor:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
        case BlendFactor::DstColor:
            return VK_BLEND_FACTOR_DST_COLOR;
        case BlendFactor::InvDstColor:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
        case BlendFactor::SrcAlpha:
            return VK_BLEND_FACTOR_SRC_ALPHA;
        case BlendFactor::InvSrcAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        case BlendFactor::DstAlpha:
            return VK_BLEND_FACTOR_DST_ALPHA;
        case BlendFactor::InvDstAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
        case BlendFactor::ConstantColor:
            return VK_BLEND_FACTOR_CONSTANT_COLOR;
        case BlendFactor::InvConstantColor:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
        case BlendFactor::ConstantAlpha:
            return VK_BLEND_FACTOR_CONSTANT_ALPHA;
        case BlendFactor::InvConstantAlpha:
            return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
        default:
            return VK_BLEND_FACTOR_ZERO;
    }
}

inline VkLogicOp ToVkLogicOp(LogicOp op) {
    switch (op) {
        case LogicOp::Clear:
            return VK_LOGIC_OP_CLEAR;
        case LogicOp::And:
            return VK_LOGIC_OP_AND;
        case LogicOp::AndReverse:
            return VK_LOGIC_OP_AND_REVERSE;
        case LogicOp::AndInverted:
            return VK_LOGIC_OP_AND_INVERTED;
        case LogicOp::Copy:
            return VK_LOGIC_OP_COPY;
        case LogicOp::CopyInverted:
            return VK_LOGIC_OP_COPY_INVERTED;
        case LogicOp::Noop:
            return VK_LOGIC_OP_NO_OP;
        case LogicOp::XOR:
            return VK_LOGIC_OP_XOR;
        case LogicOp::OR:
            return VK_LOGIC_OP_OR;
        case LogicOp::ORReverse:
            return VK_LOGIC_OP_OR_REVERSE;
        case LogicOp::ORInverted:
            return VK_LOGIC_OP_OR_INVERTED;
        case LogicOp::NOR:
            return VK_LOGIC_OP_NOR;
        case LogicOp::Equivalent:
            return VK_LOGIC_OP_EQUIVALENT;
        case LogicOp::Invert:
            return VK_LOGIC_OP_INVERT;
        default:
            return VK_LOGIC_OP_CLEAR;
    }
}

inline uint32_t ToVkColorWriteMask(uint32_t flags) {
    uint32_t mask = 0;

    if (flags & COLOR_COMPONENT_R) {
        mask |= VK_COLOR_COMPONENT_R_BIT;
    }
    if (flags & COLOR_COMPONENT_G) {
        mask |= VK_COLOR_COMPONENT_G_BIT;
    }
    if (flags & COLOR_COMPONENT_B) {
        mask |= VK_COLOR_COMPONENT_B_BIT;
    }
    if (flags & COLOR_COMPONENT_A) {
        mask |= VK_COLOR_COMPONENT_A_BIT;
    }

    return mask;
}

inline VkPrimitiveTopology ToVkPrimitiveTopology(Topology topology) {
    switch (topology) {
        case Topology::PointList:
            return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
        case Topology::LineList:
            return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
        case Topology::TriangleList:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        default:
            return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    }
}

inline VkCompareOp ToVkCompareOp(CompareOp op) {
    switch (op) {
        case CompareOp::Never:
            return VK_COMPARE_OP_NEVER;
        case CompareOp::Less:
            return VK_COMPARE_OP_LESS;
        case CompareOp::Equal:
            return VK_COMPARE_OP_EQUAL;
        case CompareOp::LessOrEqual:
            return VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareOp::Greater:
            return VK_COMPARE_OP_GREATER;
        case CompareOp::NotEqual:
            return VK_COMPARE_OP_NOT_EQUAL;
        case CompareOp::GreaterOrEqual:
            return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOp::Always:
            return VK_COMPARE_OP_ALWAYS;
        default:
            return VK_COMPARE_OP_NEVER;
    }
}

inline VkStencilOp ToVkStencilOp(StencilOp op) {
    switch (op) {
        case StencilOp::Keep:
            return VK_STENCIL_OP_KEEP;
        case StencilOp::Zero:
            return VK_STENCIL_OP_ZERO;
        case StencilOp::Replace:
            return VK_STENCIL_OP_REPLACE;
        case StencilOp::Invert:
            return VK_STENCIL_OP_INVERT;
        case StencilOp::IncrementClamp:
            return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
        case StencilOp::DecrementClamp:
            return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
        case StencilOp::IncrementWrap:
            return VK_STENCIL_OP_INCREMENT_AND_WRAP;
        case StencilOp::DecrementWrap:
            return VK_STENCIL_OP_DECREMENT_AND_WRAP;
        default:
            return VK_STENCIL_OP_KEEP;
    }
}

} // vk

#endif //DERANGED_RHI_VULKANPIPELINE_H
