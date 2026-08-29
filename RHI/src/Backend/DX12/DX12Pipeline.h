//
// Created by alan on 12/08/2026.
//

#ifndef DERANGED_RHI_DX12PIPELINE_H
#define DERANGED_RHI_DX12PIPELINE_H

#include "Pipeline.h"
#include <d3d12.h>
#include "ReleaseManager.h"
#include <unordered_map>
#include <string>

namespace dx {

class DX12Device;

class DX12GraphicsPipelineState : public GraphicsPipelineState {
public:
    DX12GraphicsPipelineState(GraphicsPipelineDesc desc, DX12Device* device);
    ~DX12GraphicsPipelineState() override;

    GraphicsPipelineDesc GetDesc() override;

    ID3D12RootSignature* GetRootSignature() { return m_RootSignature; }
    ID3D12PipelineState* GetPipelineState() { return m_PipelineState; };
    uint32_t GetDescriptorOffset(std::string name) { return m_DescriptorOffsets.at(name); }

private:
    void CreateRootSignature();
    void CreatePipeline();

private:
    DX12Device* m_Device = nullptr;
    GraphicsPipelineDesc m_Desc;
    ID3D12RootSignature* m_RootSignature = nullptr;
    ID3D12PipelineState* m_PipelineState = nullptr;
    std::unordered_map<std::string, uint32_t> m_DescriptorOffsets;

};

struct PipelineStateReleaseResource : ReleaseResourceBase {
    ID3D12RootSignature* RootSignature;
    ID3D12PipelineState* PipelineState;

    PipelineStateReleaseResource(ID3D12RootSignature* rootSignature, ID3D12PipelineState* pipelineState)
        : RootSignature(rootSignature), PipelineState(pipelineState) {}

    void Destroy() override {
        RootSignature->Release();
        PipelineState->Release();
    }
};

inline DXGI_FORMAT ToDXGIFormat(ValueType type) {
    switch (type) {
        case ValueType::Int:
            return DXGI_FORMAT_R32_SINT;
        case ValueType::Int2:
            return DXGI_FORMAT_R32G32_SINT;
        case ValueType::Int3:
            return DXGI_FORMAT_R32G32B32_SINT;
        case ValueType::Int4:
            return DXGI_FORMAT_R32G32B32A32_SINT;

        case ValueType::Float:
            return DXGI_FORMAT_R32_FLOAT;
        case ValueType::Float2:
            return DXGI_FORMAT_R32G32_FLOAT;
        case ValueType::Float3:
            return DXGI_FORMAT_R32G32B32_FLOAT;
        case ValueType::Float4:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;

        default:
            return DXGI_FORMAT_R32_SINT;
    }
}

inline D3D12_FILL_MODE ToD3D12FillMode(PolygonMode mode) {
    switch (mode) {
        case PolygonMode::Fill:
            return D3D12_FILL_MODE_SOLID;
        case PolygonMode::Line:
            return D3D12_FILL_MODE_WIREFRAME;
        default:
            return D3D12_FILL_MODE_SOLID;
    }
}

inline D3D12_CULL_MODE ToD3D12CullMode(CullMode mode) {
    switch (mode) {
        case CullMode::None:
            return D3D12_CULL_MODE_NONE;
        case CullMode::Front:
            return D3D12_CULL_MODE_FRONT;
        case CullMode::Back:
            return D3D12_CULL_MODE_BACK;
        default:
            return D3D12_CULL_MODE_NONE;
    }
}

inline D3D12_BLEND_OP ToD3D12BlendOp(BlendOp op) {
    switch (op) {
        case BlendOp::Add:
            return D3D12_BLEND_OP_ADD;
        case BlendOp::Subtract:
            return D3D12_BLEND_OP_SUBTRACT;
        case BlendOp::ReverseSubtract:
            return D3D12_BLEND_OP_REV_SUBTRACT;
        case BlendOp::Min:
            return D3D12_BLEND_OP_MIN;
        case BlendOp::Max:
            return D3D12_BLEND_OP_MAX;
        default:
            return D3D12_BLEND_OP_ADD;
    }
}

inline D3D12_BLEND ToD3D12Blend(BlendFactor factor) {
    switch (factor) {
        case BlendFactor::Zero:
            return D3D12_BLEND_ZERO;
        case BlendFactor::One:
            return D3D12_BLEND_ONE;
        case BlendFactor::SrcColor:
            return D3D12_BLEND_SRC_COLOR;
        case BlendFactor::InvSrcColor:
            return D3D12_BLEND_INV_SRC_COLOR;
        case BlendFactor::DstColor:
            return D3D12_BLEND_DEST_COLOR;
        case BlendFactor::InvDstColor:
            return D3D12_BLEND_INV_DEST_COLOR;
        case BlendFactor::SrcAlpha:
            return D3D12_BLEND_SRC_ALPHA;
        case BlendFactor::InvSrcAlpha:
            return D3D12_BLEND_INV_SRC_ALPHA;
        case BlendFactor::DstAlpha:
            return D3D12_BLEND_DEST_ALPHA;
        case BlendFactor::InvDstAlpha:
            return D3D12_BLEND_INV_DEST_ALPHA;
        case BlendFactor::ConstantColor:
            return D3D12_BLEND_BLEND_FACTOR;
        case BlendFactor::InvConstantColor:
            return D3D12_BLEND_INV_BLEND_FACTOR;
        case BlendFactor::ConstantAlpha:
            return D3D12_BLEND_ALPHA_FACTOR;
        case BlendFactor::InvConstantAlpha:
            return D3D12_BLEND_INV_ALPHA_FACTOR;
        default:
            return D3D12_BLEND_ZERO;
    }
}

inline D3D12_LOGIC_OP ToD3D12LogicOp(LogicOp op) {
    switch (op) {
        case LogicOp::Clear:
            return D3D12_LOGIC_OP_CLEAR;
        case LogicOp::And:
            return D3D12_LOGIC_OP_AND;
        case LogicOp::AndReverse:
            return D3D12_LOGIC_OP_AND_REVERSE;
        case LogicOp::AndInverted:
            return D3D12_LOGIC_OP_AND_INVERTED;
        case LogicOp::Copy:
            return D3D12_LOGIC_OP_COPY;
        case LogicOp::CopyInverted:
            return D3D12_LOGIC_OP_COPY_INVERTED;
        case LogicOp::Noop:
            return D3D12_LOGIC_OP_NOOP;
        case LogicOp::XOR:
            return D3D12_LOGIC_OP_XOR;
        case LogicOp::OR:
            return D3D12_LOGIC_OP_OR;
        case LogicOp::ORReverse:
            return D3D12_LOGIC_OP_OR_REVERSE;
        case LogicOp::ORInverted:
            return D3D12_LOGIC_OP_OR_INVERTED;
        case LogicOp::NOR:
            return D3D12_LOGIC_OP_NOR;
        case LogicOp::Equivalent:
            return D3D12_LOGIC_OP_EQUIV;
        case LogicOp::Invert:
            return D3D12_LOGIC_OP_INVERT;
        default:
            return D3D12_LOGIC_OP_CLEAR;
    }
}

inline uint8_t ToD3D12ColorWriteMask(uint8_t flags) {
    uint8_t mask = 0;

    if (flags & COLOR_COMPONENT_R) {
        mask |= D3D12_COLOR_WRITE_ENABLE_RED ;
    }
    if (flags & COLOR_COMPONENT_G) {
        mask |= D3D12_COLOR_WRITE_ENABLE_GREEN;
    }
    if (flags & COLOR_COMPONENT_B) {
        mask |= D3D12_COLOR_WRITE_ENABLE_BLUE;
    }
    if (flags & COLOR_COMPONENT_A) {
        mask |= D3D12_COLOR_WRITE_ENABLE_ALPHA;
    }

    return mask;
}

inline D3D12_PRIMITIVE_TOPOLOGY_TYPE ToD3D12PrimitiveTopologyType(Topology topology) {
    switch (topology) {
        case Topology::PointList:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
        case Topology::LineList:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
        case Topology::TriangleList:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
        default:
            return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    }
}

inline D3D12_PRIMITIVE_TOPOLOGY ToD3D12PrimitiveTopology(Topology topology) {
    switch (topology) {
        case Topology::PointList:
            return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
        case Topology::LineList:
            return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
        case Topology::TriangleList:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
        default:
            return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
    }
}

inline D3D12_COMPARISON_FUNC ToD3D12ComparisonFunc(CompareOp op) {
    switch (op) {
        case CompareOp::Never:
            return D3D12_COMPARISON_FUNC_NEVER;
        case CompareOp::Less:
            return D3D12_COMPARISON_FUNC_LESS;
        case CompareOp::Equal:
            return D3D12_COMPARISON_FUNC_EQUAL;
        case CompareOp::LessOrEqual:
            return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case CompareOp::Greater:
            return D3D12_COMPARISON_FUNC_GREATER;
        case CompareOp::NotEqual:
            return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case CompareOp::GreaterOrEqual:
            return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case CompareOp::Always:
            return D3D12_COMPARISON_FUNC_ALWAYS;
        default:
            return D3D12_COMPARISON_FUNC_NEVER;
    }
}

inline D3D12_STENCIL_OP ToD3D12StencilOp(StencilOp op) {
    switch (op) {
        case StencilOp::Keep:
            return D3D12_STENCIL_OP_KEEP;
        case StencilOp::Zero:
            return D3D12_STENCIL_OP_ZERO;
        case StencilOp::Replace:
            return D3D12_STENCIL_OP_REPLACE;
        case StencilOp::Invert:
            return D3D12_STENCIL_OP_INVERT;
        case StencilOp::IncrementClamp:
            return D3D12_STENCIL_OP_INCR_SAT;
        case StencilOp::DecrementClamp:
            return D3D12_STENCIL_OP_DECR_SAT;
        case StencilOp::IncrementWrap:
            return D3D12_STENCIL_OP_INCR;
        case StencilOp::DecrementWrap:
            return D3D12_STENCIL_OP_DECR;
        default:
            return D3D12_STENCIL_OP_KEEP;
    }
}

inline D3D12_DESCRIPTOR_RANGE_TYPE ToD3D12DescriptorRangeType(D3D_SHADER_INPUT_TYPE  type) {
    switch (type) {
        case D3D_SIT_CBUFFER:
            return D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
        case D3D_SIT_TEXTURE:
            return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
        default:
            return D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    }
}

} // dx

#endif //DERANGED_RHI_DX12PIPELINE_H
