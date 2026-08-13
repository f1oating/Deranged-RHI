//
// Created by alan on 13/08/2026.
//

#include "Backend/DX12/DX12Resource.h"

#include "Backend/DX12/DX12Device.h"

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
        .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN
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
    m_Device->ReleaseResource(new TextureReleaseResource(m_Resource));
}

DX12TextureView::DX12TextureView(TextureViewDesc desc, DX12Device* device) {
    m_Desc = desc;
    m_Device = device;
}

DX12TextureView::~DX12TextureView() {

}
