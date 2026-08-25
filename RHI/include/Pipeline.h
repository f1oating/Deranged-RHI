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

enum class CompareOp {
    Never,
    Less,
    Equal,
    LessOrEqual,
    Greater,
    NotEqual,
    GreaterOrEqual,
    Always
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
    PolygonMode Polygon;
    CullMode Cull;
    FrontFace Face;
    float DepthBiasClamp;
    float DepthBiasConstant;
    float DepthBiasSlope;
};

struct DepthStencilDesc {
    CompareOp DepthCompare;
    StencilOp Front;
    StencilOp Back;
};

struct BlendAttachmentDesc {
    bool BlendEnable;
    BlendFactor SrcColorBlend;
    BlendFactor DstColorBlend;
    BlendFactor SrcAlphaBlend;
    BlendFactor DstAlphaBlend;
    BlendOp ColorBlend;
    BlendOp AlphaBlend;
    uint32_t ColorWriteMask;
};

struct BlendDesc {
    bool LogicOpEnable;
    LogicOp Logic;
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
    TextureFormat DepthStencilFormat;
    Topology PrimitiveTopology;
};

class GraphicsPipelineState {
public:
    virtual ~GraphicsPipelineState() = default;

};

#endif //DERANGED_RHI_PIPELINE_H
