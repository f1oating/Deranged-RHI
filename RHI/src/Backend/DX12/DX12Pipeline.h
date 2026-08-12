//
// Created by alan on 12/08/2026.
//

#ifndef DERANGED_RHI_DX12PIPELINE_H
#define DERANGED_RHI_DX12PIPELINE_H

#include "Pipeline.h"
#include <d3d12.h>
#include "ReleaseManager.h"

class DX12Device;

class DX12GraphicsPipelineState : public GraphicsPipelineState {
public:
    DX12GraphicsPipelineState(GraphicsPipelineDesc desc, DX12Device* device);
    ~DX12GraphicsPipelineState() override;

    ID3D12RootSignature* GetRootSignature() { return m_RootSignature; }
    ID3D12PipelineState* GetPipelineState() { return m_PipelineState; };

private:
    void CreateRootSignature();
    void CreatePipeline();

private:
    DX12Device* m_Device = nullptr;
    GraphicsPipelineDesc m_Desc;
    ID3D12RootSignature* m_RootSignature = nullptr;
    ID3D12PipelineState* m_PipelineState = nullptr;

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

#endif //DERANGED_RHI_DX12PIPELINE_H
