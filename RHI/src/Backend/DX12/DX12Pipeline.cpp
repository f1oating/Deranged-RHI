//
// Created by alan on 12/08/2026.
//

#include "Backend/DX12/DX12Pipeline.h"
#include "Backend/DX12/DX12Device.h"

DX12GraphicsPipelineState::DX12GraphicsPipelineState(GraphicsPipelineDesc desc, DX12Device* device) {
    m_Device = device;
    m_Desc = desc;

    CreateRootSignature();
    CreatePipeline();
}

DX12GraphicsPipelineState::~DX12GraphicsPipelineState() {
    if (m_RootSignature && m_PipelineState) {
        m_Device->ReleaseResource(new PipelineStateReleaseResource(m_RootSignature, m_PipelineState));
    }
}

void DX12GraphicsPipelineState::CreateRootSignature() {
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {
        .NumParameters = 0,
        .pParameters = nullptr,
        .NumStaticSamplers = 0,
        .pStaticSamplers = nullptr
    };

    ID3DBlob* serializedSignature = nullptr;
    ID3DBlob* diagnosticBlob = nullptr;
    HRESULT hr = D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0,
        &serializedSignature, &diagnosticBlob);
    if (diagnosticBlob) {
        printf("%s", (const char*)diagnosticBlob->GetBufferPointer());
    }

    hr = m_Device->GetDX12Device()->CreateRootSignature(0, serializedSignature->GetBufferPointer(),
        serializedSignature->GetBufferSize(), IID_PPV_ARGS(&m_RootSignature));
}

void DX12GraphicsPipelineState::CreatePipeline() {
    DXGI_SAMPLE_DESC sampleDesc = {
        .Count = 1,
        .Quality = 0
    };

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {
        .pInputElementDescs = nullptr,
        .NumElements = 0
    };

    D3D12_RASTERIZER_DESC rasterizerDesc = {
        .FillMode = D3D12_FILL_MODE_SOLID,
        .CullMode = D3D12_CULL_MODE_NONE,
        .FrontCounterClockwise = false,
        .DepthClipEnable = false,
        .AntialiasedLineEnable = false
    };

    D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {
        .BlendEnable = false,
        .LogicOpEnable = false,
        .RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL
    };

    D3D12_BLEND_DESC blendDesc = {
        .AlphaToCoverageEnable = false,
        .IndependentBlendEnable = false
    };
    blendDesc.RenderTarget[0] = renderTargetBlendDesc;

    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {
        .DepthEnable = false,
        .StencilEnable = false
    };

    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc = {
        .pRootSignature = m_RootSignature,
        .VS = { m_Desc.VertexShader.Data, m_Desc.VertexShader.Size },
        .PS = { m_Desc.FragmentShader.Data, m_Desc.FragmentShader.Size },
        .BlendState = blendDesc,
        .SampleMask = 0xffffffff,
        .RasterizerState = rasterizerDesc,
        .DepthStencilState = depthStencilDesc,
        .InputLayout = inputLayoutDesc,
        .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
        .NumRenderTargets = 1,
        .SampleDesc = sampleDesc
    };
    graphicsPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_B8G8R8A8_UNORM;

    HRESULT hr = m_Device->GetDX12Device()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&m_PipelineState));
}
