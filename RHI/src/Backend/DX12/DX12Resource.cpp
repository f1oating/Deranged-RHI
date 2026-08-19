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

    D3D12_RESOURCE_DESC resourceDesc = {
        .Dimension = ToD3D12ResourceDimension(m_Desc.Type),
        .Width = m_Desc.Width,
        .Height = m_Desc.Height,
        .DepthOrArraySize = 1,
        .MipLevels = (uint16_t)m_Desc.MipLevels,
        .Format = ToDXGIFormat(m_Desc.Format),
        .SampleDesc = { 1, 0 },
        .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
        .Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET
    };

    HRESULT hr = m_Device->GetDX12Device()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE,
        &resourceDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&m_Resource));
}

DX12Texture::DX12Texture(TextureDesc desc, ID3D12Resource* res, DX12Device* device) {
    m_Desc = desc;
    m_Resource = res;
    m_Device = device;
}

DX12Texture::~DX12Texture() {
    if (m_RTV) {
        delete m_RTV;
    }
    m_Device->ReleaseResource(new TextureReleaseResource(m_Resource));
}

TextureView* DX12Texture::GetRTV() {
    if (!m_RTV) {
        TextureViewDesc desc = {
            .Tex = this,
            .Format = TextureFormat::TEXTURE_FORMAT_B8G8R8A8_UNORM
        };
        m_RTV = new DX12TextureView(desc, m_Device);
    }
    return m_RTV;
}

DX12TextureView::DX12TextureView(TextureViewDesc desc, DX12Device* device) {
    m_Desc = desc;
    m_Device = device;
    m_Allocation = m_Device->GetRTVAllocator()->Allocate(1);

    CreateRTV();
}

DX12TextureView::~DX12TextureView() {
    if (!m_Allocation.IsNull()) {
        m_Device->ReleaseResource(new DescriptorAllocationReleaseResource(m_Device->GetRTVAllocator(), m_Allocation));
    }
}

void DX12TextureView::CreateRTV() {
    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {
        .Format = ToDXGIFormat(m_Desc.Format),
        .ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D
    };

    DX12Texture* dxTexture = static_cast<DX12Texture*>(m_Desc.Tex);

    m_Device->GetDX12Device()->CreateRenderTargetView(dxTexture->GetDX12Resource(), &rtvDesc, m_Allocation.GetCPUHandle(0));
}

} // dx
