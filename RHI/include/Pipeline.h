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

enum class ValueType {
    Int, Int2, Int3, Int4,
    Float, Float2, Float3, Float4
};

struct InputElementDesc {
    std::string Name;
    ValueType Type;
};

struct VertexInputDesc {
    std::vector<InputElementDesc> InputElements;
};

struct RasterizationDesc {

};

struct MultisampleDesc {

};

struct DepthStencilDesc {

};

struct BlendDesc {

};

struct GraphicsPipelineDesc {
    Shader VertexShader;
    Shader FragmentShader;
    VertexInputDesc VertexInput;
    RasterizationDesc Rasterization;
    MultisampleDesc Multisample;
    DepthStencilDesc DepthStencil;
    BlendDesc Blend;
    std::vector<TextureFormat> ColorFormats;
    TextureFormat DepthStencilFormat;
};

class GraphicsPipelineState {
public:
    virtual ~GraphicsPipelineState() = default;

};

#endif //DERANGED_RHI_PIPELINE_H
