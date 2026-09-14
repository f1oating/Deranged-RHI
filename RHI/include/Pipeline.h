//
// Created by alan on 11/08/2026.
//

#ifndef DERANGED_RHI_PIPELINE_H
#define DERANGED_RHI_PIPELINE_H

#include <cstdint>
#include <vector>
#include "Resource.h"
#include <string>

struct Shader {
    void* Data;
    uint64_t Size;
};

enum class Topology {
    PointList,
    LineList,
    TriangleList
};

enum class ValueType {
    Int, Int2, Int3, Int4,
    Float, Float2, Float3, Float4
};

enum class PolygonMode {
    Fill, Line
};

enum class CullMode {
    None, Front, Back
};

enum class FrontFace {
    CW, CCW
};

enum class BlendOp {
    Add,
    Subtract,
    ReverseSubtract,
    Min,
    Max
};

enum class BlendFactor {
    Zero,
    One,
    SrcColor,
    InvSrcColor,
    DstColor,
    InvDstColor,
    SrcAlpha,
    InvSrcAlpha,
    DstAlpha,
    InvDstAlpha,
    ConstantColor,
    InvConstantColor,
    ConstantAlpha,
    InvConstantAlpha
};

enum class LogicOp {
    Clear,
    And,
    AndReverse,
    AndInverted,
    Copy,
    CopyInverted,
    Set,
    Noop,
    XOR,
    OR,
    ORReverse,
    ORInverted,
    NOR,
    Equivalent,
    Invert
};

enum ColorWriteFlags : uint8_t {
    COLOR_COMPONENT_R = 1 << 0,
    COLOR_COMPONENT_G = 1 << 1,
    COLOR_COMPONENT_B = 1 << 2,
    COLOR_COMPONENT_A = 1 << 3
};

enum class StencilOp {
    Keep,
    Zero,
    Replace,
    Invert,
    IncrementClamp,
    DecrementClamp,
    IncrementWrap,
    DecrementWrap,
};

struct InputElementDesc {
    std::string Name;
    ValueType Type;
};

struct VertexInputDesc {
    std::vector<InputElementDesc> InputElements;
};

struct RasterizationDesc {
    PolygonMode Polygon = PolygonMode::Fill;
    CullMode Cull = CullMode::None;
    FrontFace Face = FrontFace::CW;
    float DepthBiasClamp = 0.0f;
    float DepthBiasConstant = 0.0f;
    float DepthBiasSlope = 0.0f;
};

struct StencilStateDesc {
    StencilOp DepthFail = StencilOp::Keep;
    StencilOp Fail = StencilOp::Keep;
    StencilOp Pass = StencilOp::Keep;
    CompareOp StencilFunc = CompareOp::Never;
};

struct DepthStencilDesc {
    bool DepthEnable = false;
    bool StencilEnable = false;
    bool DepthWriteEnable = false;
    CompareOp DepthCompare = CompareOp::Never;
    StencilStateDesc Front;
    StencilStateDesc Back;
    uint8_t StencilReadMask = 0xff;
    uint8_t StencilWriteMask = 0xff;
};

struct BlendAttachmentDesc {
    bool BlendEnable = false;
    BlendFactor SrcColorBlend = BlendFactor::Zero;
    BlendFactor DstColorBlend = BlendFactor::Zero;
    BlendFactor SrcAlphaBlend = BlendFactor::Zero;
    BlendFactor DstAlphaBlend = BlendFactor::Zero;
    BlendOp ColorBlend = BlendOp::Add;
    BlendOp AlphaBlend = BlendOp::Add;
    uint32_t ColorWriteMask = COLOR_COMPONENT_R | COLOR_COMPONENT_G | COLOR_COMPONENT_B | COLOR_COMPONENT_A;
};

struct BlendDesc {
    bool LogicOpEnable = false;
    LogicOp Logic = LogicOp::Copy;
    std::vector<BlendAttachmentDesc> ColorAttachments;
};

struct GraphicsPipelineDesc {
    Shader VertexShader;
    Shader FragmentShader;
    VertexInputDesc VertexInput;
    RasterizationDesc Rasterization;
    DepthStencilDesc DepthStencil;
    BlendDesc Blend;
    std::vector<TextureFormat> ColorFormats;
    TextureFormat DepthStencilFormat = TextureFormat::Unknown;
    Topology PrimitiveTopology = Topology::TriangleList;
};

class GraphicsPipelineState {
public:
    virtual ~GraphicsPipelineState() = default;

    virtual GraphicsPipelineDesc GetDesc() = 0;

};

#endif //DERANGED_RHI_PIPELINE_H
