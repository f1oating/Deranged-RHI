//
// Created by alan on 12/08/2026.
//

#include "Backend/DX12/DX12Pipeline.h"
#include "Backend/DX12/DX12Device.h"
#include "Backend/DX12/DX12Resource.h"

namespace dx {

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

GraphicsPipelineDesc DX12GraphicsPipelineState::GetDesc() {
    return m_Desc;
}

void DX12GraphicsPipelineState::CreateRootSignature() {
    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {
        .NumParameters = 0,
        .pParameters = nullptr,
        .NumStaticSamplers = 0,
        .pStaticSamplers = nullptr,
        .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
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

    std::vector<D3D12_INPUT_ELEMENT_DESC> inputElements;
    for (int i = 0; i < m_Desc.VertexInput.InputElements.size(); i++) {
        D3D12_INPUT_ELEMENT_DESC inputElement = {
            .SemanticName = m_Desc.VertexInput.InputElements[i].Name.c_str(),
            .SemanticIndex = 0,
            .Format = ToDXGIFormat(m_Desc.VertexInput.InputElements[i].Type),
            .InputSlot = 0,
            .AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT,
            .InputSlotClass = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,
            .InstanceDataStepRate = 0
        };
        inputElements.push_back(inputElement);
    }

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc = {
        .pInputElementDescs = inputElements.data(),
        .NumElements = (uint32_t)inputElements.size()
    };

    D3D12_RASTERIZER_DESC rasterizerDesc = {
        .FillMode = ToD3D12FillMode(m_Desc.Rasterization.Polygon),
        .CullMode = ToD3D12CullMode(m_Desc.Rasterization.Cull),
        .FrontCounterClockwise = m_Desc.Rasterization.Face == FrontFace::CW ? false : true,
        .DepthBias = (int)m_Desc.Rasterization.DepthBiasConstant,
        .DepthBiasClamp = m_Desc.Rasterization.DepthBiasClamp,
        .SlopeScaledDepthBias = m_Desc.Rasterization.DepthBiasSlope,
        .DepthClipEnable = true,
        .MultisampleEnable = false,
        .AntialiasedLineEnable = false,
        .ForcedSampleCount = 0,
        .ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF
    };

    D3D12_BLEND_DESC blendDesc = {
        .AlphaToCoverageEnable = false,
        .IndependentBlendEnable = false
    };

    for (int i = 0; i < m_Desc.Blend.ColorAttachments.size(); i++) {
        D3D12_RENDER_TARGET_BLEND_DESC renderTargetBlendDesc = {
            .BlendEnable = m_Desc.Blend.ColorAttachments[i].BlendEnable,
            .LogicOpEnable = m_Desc.Blend.LogicOpEnable,
            .SrcBlend = ToD3D12Blend(m_Desc.Blend.ColorAttachments[i].SrcColorBlend),
            .DestBlend = ToD3D12Blend(m_Desc.Blend.ColorAttachments[i].DstColorBlend),
            .BlendOp = ToD3D12BlendOp(m_Desc.Blend.ColorAttachments[i].ColorBlend),
            .SrcBlendAlpha = ToD3D12Blend(m_Desc.Blend.ColorAttachments[i].SrcAlphaBlend),
            .DestBlendAlpha = ToD3D12Blend(m_Desc.Blend.ColorAttachments[i].DstAlphaBlend),
            .BlendOpAlpha = ToD3D12BlendOp(m_Desc.Blend.ColorAttachments[i].AlphaBlend),
            .LogicOp = ToD3D12LogicOp(m_Desc.Blend.Logic),
            .RenderTargetWriteMask = ToD3D12ColorWriteMask(m_Desc.Blend.ColorAttachments[i].ColorWriteMask)
        };

        blendDesc.RenderTarget[i] = renderTargetBlendDesc;
    }

    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {
        .DepthFunc = ToD3D12ComparisonFunc(m_Desc.DepthStencil.DepthCompare),
        .FrontFace = ToD3D12StencilOp(m_Desc.DepthStencil.Front),
        .BackFace = ToD3D12StencilOp(m_Desc.DepthStencil.Back)
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
        .PrimitiveTopologyType = ToD3D12PrimitiveTopologyType(m_Desc.PrimitiveTopology),
        .NumRenderTargets = (uint32_t)m_Desc.ColorFormats.size(),
        .SampleDesc = sampleDesc
    };

    for (int i = 0; i < m_Desc.ColorFormats.size(); i++) {
        graphicsPipelineStateDesc.RTVFormats[0] = ToDXGIFormat(m_Desc.ColorFormats[i]);
    }
    graphicsPipelineStateDesc.DSVFormat = ToDXGIFormat(m_Desc.DepthStencilFormat);

    HRESULT hr = m_Device->GetDX12Device()->CreateGraphicsPipelineState(&graphicsPipelineStateDesc, IID_PPV_ARGS(&m_PipelineState));
}

} // dx
