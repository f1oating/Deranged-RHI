//
// Created by alan on 13/08/2026.
//

#include "Backend/DX12/DX12Resource.h"

#include "Backend/DX12/DX12Device.h"

namespace dx {

DX12Texture::DX12Texture(TextureDesc desc, DX12Device* device) {
    m_Desc = desc;
    m_Device = device;

    D3D12_HEAP_PROPERTIES heapProps = {
        .Type = D3D12_HEAP_TYPE_DEFAULT
    };

    D3D12_RESOURCE_DESC1 resourceDesc = {
        .Dimension = ToD3D12ResourceDimension(m_Desc.Type),
        .Width = m_Desc.Width,
        .Height = m_Desc.Height,
        .DepthOrArraySize = 1,
        .MipLevels = (uint16_t)m_Desc.MipLevels,
        .Format = ToDXGIFormat(m_Desc.Format),
        .SampleDesc = { 1, 0 },
        .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
        .Flags = ToD3D12TexResourceFlags(desc.BindFlags)
    };

    HRESULT hr = m_Device->GetDX12Device()->CreateCommittedResource3(&heapProps, D3D12_HEAP_FLAG_NONE,
        &resourceDesc, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr,
        nullptr, 0, nullptr, IID_PPV_ARGS(&m_Resource));
}

DX12Texture::DX12Texture(TextureDesc desc, ID3D12Resource* res, DX12Device* device) {
    m_Desc = desc;
    m_Resource = res;
    m_Device = device;
    m_Layout = ImageLayout::Present;
}

DX12Texture::~DX12Texture() {
    if (m_RTV) {
        delete m_RTV;
    }
    if (m_DSV) {
        delete m_DSV;
    }
    if (m_SRV) {
        delete m_SRV;
    }
    m_Device->ReleaseResource(new TextureReleaseResource(m_Resource));
}

RenderTargetView* DX12Texture::GetRTV() {
    if (!m_RTV) {
        m_RTV = new DX12RenderTargetView(this, m_Device);
    }
    return m_RTV;
}

DepthStencilView* DX12Texture::GetDSV() {
    if (!m_DSV) {
        m_DSV = new DX12DepthStencilView(this, m_Device);
    }
    return m_DSV;
}

ShaderResourceView* DX12Texture::GetSRV() {
    if (!m_RTV) {
        m_SRV = new DX12ShaderResourceView(this, m_Device);
    }
    return m_SRV;
}

TextureDesc DX12Texture::GetDesc() {
    return m_Desc;
}

DX12RenderTargetView::DX12RenderTargetView(DX12Texture* texture, DX12Device* device) {
    m_Device = device;
    m_Texture = texture;
    m_Allocation = m_Device->GetRTVAllocator()->Allocate(1);

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {
        .Format = ToDXGIFormat(m_Texture->GetDesc().Format),
        .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D
    };

    m_Device->GetDX12Device()->CreateRenderTargetView(m_Texture->GetDX12Resource(), &rtvDesc, m_Allocation.GetCPUHandle(0));
}

DX12RenderTargetView::~DX12RenderTargetView() {
    if (!m_Allocation.IsNull()) {
        m_Device->ReleaseResource(new DescriptorAllocationReleaseResource(m_Device->GetRTVAllocator(), m_Allocation));
    }
}

DX12DepthStencilView::DX12DepthStencilView(DX12Texture* texture, DX12Device* device) {
    m_Device = device;
    m_Texture = texture;
    m_Allocation = m_Device->GetDSVAllocator()->Allocate(1);

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {
        .Format = ToDXGIFormat(m_Texture->GetDesc().Format),
        .ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D
    };

    m_Device->GetDX12Device()->CreateDepthStencilView(m_Texture->GetDX12Resource(), &dsvDesc, m_Allocation.GetCPUHandle(0));
}

DX12DepthStencilView::~DX12DepthStencilView() {
    if (!m_Allocation.IsNull()) {
        m_Device->ReleaseResource(new DescriptorAllocationReleaseResource(m_Device->GetDSVAllocator(), m_Allocation));
    }
}

DX12ShaderResourceView::DX12ShaderResourceView(DX12Texture* texture, DX12Device* device) {
    m_Device = device;
    m_Texture = texture;

    D3D12_SHADER_RESOURCE_VIEW_DESC view = {
        .Format = ToDXGIFormat(m_Texture->GetDesc().Format),
        .ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D
    };

    m_View = view;
}

DX12ShaderResourceView::~DX12ShaderResourceView() {

}

DX12Buffer::DX12Buffer(BufferDesc desc, DX12Device* device) {
    m_Desc = desc;
    m_Device = device;

    CreateResource();
}

DX12Buffer::~DX12Buffer() {
    if (m_Desc.Usage == BufferUsage::Dynamic) {
        m_Resource->Unmap(0, nullptr);
    }
    m_Device->ReleaseResource(new BufferReleaseResource(m_Resource));
}

void* DX12Buffer::Map() {
    m_Offset = m_Device->GetRingBuffer()->Allocate(m_Desc.Size);
    return (uint8_t*)m_Mapped + m_Offset;
}

BufferDesc DX12Buffer::GetDesc() {
    return m_Desc;
}

void DX12Buffer::CreateResource() {
    D3D12_RESOURCE_DESC1 resourceDesc = {
        .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
        .Width = m_Desc.Size,
        .Height = 1,
        .DepthOrArraySize = 1,
        .MipLevels = 1,
        .Format = DXGI_FORMAT_UNKNOWN,
        .SampleDesc = { 1, 0 },
        .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
        .Flags = ToD3D12BufResourceFlags(m_Desc.BindFlags)
    };

    HRESULT hr = 0;
    if (m_Desc.Usage == BufferUsage::Dynamic) {
        m_Device->GetDX12Device()->CreatePlacedResource2(m_Device->GetRingBuffer()->GetDX12Heap(), 0,
            &resourceDesc, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, 0, nullptr, IID_PPV_ARGS(&m_Resource));
        hr = m_Resource->Map(0, nullptr, &m_Mapped);
        return;
    }

    D3D12_HEAP_PROPERTIES heapProps = {
        .Type = m_Desc.Usage == BufferUsage::Default ? D3D12_HEAP_TYPE_DEFAULT : D3D12_HEAP_TYPE_UPLOAD
    };

    hr = m_Device->GetDX12Device()->CreateCommittedResource3(&heapProps, D3D12_HEAP_FLAG_NONE,
        &resourceDesc, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr,
        nullptr, 0, nullptr, IID_PPV_ARGS(&m_Resource));
}

DX12Sampler::DX12Sampler(SamplerDesc desc, DX12Device* device) {
    m_Device = device;
    m_Desc = desc;

    D3D12_SAMPLER_DESC samplerDesc = {
        .Filter = ToD3D12Filter(m_Desc.Filtering),
        .AddressU = ToD3D12TextureAddressMode(m_Desc.AddressU),
        .AddressV = ToD3D12TextureAddressMode(m_Desc.AddressV),
        .AddressW = ToD3D12TextureAddressMode(m_Desc.AddressW),
        .MipLODBias = m_Desc.MipLodBias,
        .MaxAnisotropy = m_Desc.MaxAnisotropy,
        .ComparisonFunc = ToD3D12ComparisonFunc(m_Desc.Compare),
        .BorderColor = { 0.0f, 0.0f, 0.0f, 1.0f },
        .MinLOD = m_Desc.MinLod,
        .MaxLOD = m_Desc.MaxLod,
    };
    m_Sampler = samplerDesc;
}

DX12Sampler::~DX12Sampler() {

}

SamplerDesc DX12Sampler::GetDesc() {
    return m_Desc;
}

} // dx
