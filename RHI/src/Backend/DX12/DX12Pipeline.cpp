//
// Created by alan on 12/08/2026.
//

#include "Backend/DX12/DX12Pipeline.h"
#include "Backend/DX12/DX12Device.h"
#include "Backend/DX12/DX12Resource.h"
#include <d3d12shader.h>
#include <dxcapi.h>
#include <set>

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

void DX12GraphicsPipelineState::ReflexShader(Shader shader, std::vector<D3D12_DESCRIPTOR_RANGE>& descriptorRanges) {
    IDxcUtils* utils = nullptr;
    DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils));

    DxcBuffer buffer = {
        .Ptr = shader.Data,
        .Size = shader.Size,
        .Encoding = 0,
    };

    ID3D12ShaderReflection* shaderReflection = nullptr;
    utils->CreateReflection(&buffer, IID_PPV_ARGS(&shaderReflection));

    D3D12_SHADER_DESC desc{};
    shaderReflection->GetDesc(&desc);

    for (int i = 0; i < desc.BoundResources; i++) {
        D3D12_SHADER_INPUT_BIND_DESC shaderInputBindDesc = {};
        shaderReflection->GetResourceBindingDesc(i, &shaderInputBindDesc);

        D3D12_DESCRIPTOR_RANGE descriptorRange = {
            .RangeType = ToD3D12DescriptorRangeType(shaderInputBindDesc.Type),
            .NumDescriptors = shaderInputBindDesc.BindCount,
            .BaseShaderRegister = shaderInputBindDesc.BindPoint,
            .RegisterSpace = shaderInputBindDesc.Space,
            .OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND
        };

        std::string name = shaderInputBindDesc.Name;
        if (!m_DescriptorOffsets.contains(name)) {
            descriptorRanges.push_back(descriptorRange);
            m_DescriptorOffsets.emplace(name.substr(0, name.find('_')), i);
            m_DescriptorsState.insert({ (uint32_t)i, { ToDescriptorType(shaderInputBindDesc.Type) } });
        }
    }

    shaderReflection->Release();
    utils->Release();
}

void DX12GraphicsPipelineState::CreateRootSignature() {
    std::vector<D3D12_DESCRIPTOR_RANGE> descriptorRanges;

    if (m_Desc.VertexShader.Data) ReflexShader(m_Desc.VertexShader, descriptorRanges);
    if (m_Desc.FragmentShader.Data) ReflexShader(m_Desc.FragmentShader, descriptorRanges);

    D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable = {
        .NumDescriptorRanges = (uint32_t)descriptorRanges.size(),
        .pDescriptorRanges = descriptorRanges.data()
    };

    D3D12_ROOT_PARAMETER rootParameter = {
        .ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
        .DescriptorTable = descriptorTable,
        .ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL
    };

    D3D12_ROOT_SIGNATURE_DESC rootSignatureDesc = {
        .NumParameters = 1,
        .pParameters = &rootParameter,
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

    D3D12_DEPTH_STENCILOP_DESC frontState = {
        .StencilFailOp = ToD3D12StencilOp(m_Desc.DepthStencil.Front.Fail),
        .StencilDepthFailOp = ToD3D12StencilOp(m_Desc.DepthStencil.Front.DepthFail),
        .StencilPassOp = ToD3D12StencilOp(m_Desc.DepthStencil.Front.Pass),
        .StencilFunc = ToD3D12ComparisonFunc(m_Desc.DepthStencil.Front.StencilFunc)
    };

    D3D12_DEPTH_STENCILOP_DESC backState = {
        .StencilFailOp = ToD3D12StencilOp(m_Desc.DepthStencil.Back.Fail),
        .StencilDepthFailOp = ToD3D12StencilOp(m_Desc.DepthStencil.Back.DepthFail),
        .StencilPassOp = ToD3D12StencilOp(m_Desc.DepthStencil.Back.Pass),
        .StencilFunc = ToD3D12ComparisonFunc(m_Desc.DepthStencil.Back.StencilFunc)
    };

    D3D12_DEPTH_STENCIL_DESC depthStencilDesc = {
        .DepthEnable = m_Desc.DepthStencil.DepthEnable,
        .DepthWriteMask = m_Desc.DepthStencil.DepthWriteEnable ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO,
        .DepthFunc = ToD3D12ComparisonFunc(m_Desc.DepthStencil.DepthCompare),
        .StencilEnable = m_Desc.DepthStencil.StencilEnable,
        .StencilReadMask = m_Desc.DepthStencil.StencilReadMask,
        .StencilWriteMask = m_Desc.DepthStencil.StencilWriteMask,
        .FrontFace = frontState,
        .BackFace = backState
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
