//
// Created by alan on 11/08/2026.
//

#ifndef DERANGED_RHI_PIPELINE_H
#define DERANGED_RHI_PIPELINE_H

#include <cstdint>

struct Shader {
    void* Data;
    uint64_t Size;
};

struct GraphicsPipelineDesc {
    Shader VertexShader;
    Shader FragmentShader;
};

class GraphicsPipelineState {
public:
    virtual ~GraphicsPipelineState() = default;

};

#endif //DERANGED_RHI_PIPELINE_H
