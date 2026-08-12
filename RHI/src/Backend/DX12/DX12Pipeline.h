//
// Created by alan on 12/08/2026.
//

#ifndef DERANGED_RHI_DX12PIPELINE_H
#define DERANGED_RHI_DX12PIPELINE_H

#include "Pipeline.h"

class DX12Device;

class DX12GraphicsPipelineState : public GraphicsPipelineState {
public:
    DX12GraphicsPipelineState(GraphicsPipelineDesc desc, DX12Device* device);
    ~DX12GraphicsPipelineState() override;

private:
    DX12Device* m_Device = nullptr;
    GraphicsPipelineDesc m_Desc;

};

#endif //DERANGED_RHI_DX12PIPELINE_H
