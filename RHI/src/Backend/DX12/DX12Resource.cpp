//
// Created by alan on 13/08/2026.
//

#include "Backend/DX12/DX12Resource.h"
#include <spdlog/spdlog.h>
#include "Backend/DX12/DX12Device.h"

namespace dx {

DX12Texture::DX12Texture(TextureDesc desc, DX12Device* device) {
    m_Desc = desc;
    m_Device = device;

    D3D12_HEAP_PROPERTIES heapProps = {
        .Type = D3D12_HEAP_TYPE_DEFAULT
    };

    D3D12_RESOURCE_DESC1 resourceDesc = {
        .Dimension = ToD3D12ResourceDimension(m_Desc.Dimension),
        .Width = m_Desc.Width,
        .Height = m_Desc.Height,
        .DepthOrArraySize = (uint16_t)m_Desc.ArrayLayers,
        .MipLevels = (uint16_t)m_Desc.MipLevels,
        .Format = ToDXGIFormat(m_Desc.Format),
        .SampleDesc = { 1, 0 },
        .Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN,
        .Flags = ToD3D12TexResourceFlags(desc.BindFlags)
    };

    HRESULT hr = m_Device->GetDX12Device()->CreateCommittedResource3(&heapProps, D3D12_HEAP_FLAG_NONE,
        &resourceDesc, D3D12_BARRIER_LAYOUT_UNDEFINED, nullptr, nullptr, 0,
        nullptr, IID_PPV_ARGS(&m_Resource));

    spdlog::info("DX12Texture Created.");
}

DX12Texture::DX12Texture(TextureDesc desc, ID3D12Resource* res, DX12Device* device) {
    m_Desc = desc;
    m_Resource = res;
    m_Device = device;
    m_Layout = TextureLayout::Present;

    spdlog::info("DX12Texture Created.");
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

    spdlog::info("DX12Texture Destroyed.");
}

TextureView* DX12Texture::GetRTV() {
    if (!m_RTV) {
        TextureViewDesc rtvDesc = {
            .Tex = this,
            .Type = TextureViewType::RTV
        };
        m_RTV = new DX12TextureView(rtvDesc, m_Device);
    }
    return m_RTV;
}

TextureView* DX12Texture::GetDSV() {
    if (!m_DSV) {
        TextureViewDesc dsvDesc = {
            .Tex = this,
            .Type = TextureViewType::DSV
        };
        m_DSV = new DX12TextureView(dsvDesc, m_Device);
    }
    return m_DSV;
}

TextureView* DX12Texture::GetSRV() {
    if (!m_SRV) {
        TextureViewDesc srvDesc = {
            .Tex = this,
            .Type = TextureViewType::SRV
        };
        m_SRV = new DX12TextureView(srvDesc, m_Device);
    }
    return m_SRV;
}

TextureDesc DX12Texture::GetDesc() {
    return m_Desc;
}

DX12TextureView::DX12TextureView(TextureViewDesc desc, DX12Device* device) {
    m_Device = device;
    m_Desc = desc;

    switch (m_Desc.Type) {
        case TextureViewType::RTV:
            CreateRTV();
            break;
        case TextureViewType::DSV:
            CreateDSV();
            break;
        case TextureViewType::SRV:
            CreateSRV();
            break;
        default:
            CreateSRV();
    }

    spdlog::info("DX12TextureView Created.");
}

DX12TextureView::~DX12TextureView() {
    if (!m_Allocation.IsNull()) {
        switch (m_Desc.Type) {
            case TextureViewType::RTV:
                m_Device->ReleaseResource(new DescriptorAllocationReleaseResource(m_Device->GetRTVAllocator(), m_Allocation));
                break;
            case TextureViewType::DSV:
                m_Device->ReleaseResource(new DescriptorAllocationReleaseResource(m_Device->GetDSVAllocator(), m_Allocation));
                break;
            case TextureViewType::SRV:
                m_Device->ReleaseResource(new DescriptorAllocationReleaseResource(m_Device->GetSRVAllocator(), m_Allocation));
                break;
            default:
                m_Device->ReleaseResource(new DescriptorAllocationReleaseResource(m_Device->GetSRVAllocator(), m_Allocation));
        }
    }

    spdlog::info("DX12TextureView Destroyed.");
}

void DX12TextureView::CreateRTV() {
    m_Allocation = m_Device->GetRTVAllocator()->Allocate(1);

    DX12Texture* vkTex = static_cast<DX12Texture*>(m_Desc.Tex);

    D3D12_RENDER_TARGET_VIEW_DESC rtvDesc = {
        .Format = ToDXGIFormat(vkTex->GetDesc().Format),
        .ViewDimension = ToD3D12RTVDimension(m_Desc.Dimension)
    };

    if (m_Desc.Dimension == TextureViewDimension::Texture1D) {
        rtvDesc.Texture1D = {
            .MipSlice = 0
        };
    } else if (m_Desc.Dimension == TextureViewDimension::Texture1DArray) {
        rtvDesc.Texture1DArray = {
            .MipSlice = 0,
            .FirstArraySlice = m_Desc.BaseArrayLayer,
            .ArraySize = m_Desc.ArrayLayers
        };
    } else if (m_Desc.Dimension == TextureViewDimension::Texture2D) {
        rtvDesc.Texture2D = {
            .MipSlice = 0,
            .PlaneSlice = 0
        };
    } else if (m_Desc.Dimension == TextureViewDimension::Texture2DArray) {
        rtvDesc.Texture2DArray = {
            .MipSlice = 0,
            .FirstArraySlice = m_Desc.BaseArrayLayer,
            .ArraySize = m_Desc.ArrayLayers,
            .PlaneSlice = 0
        };
    } else if (m_Desc.Dimension == TextureViewDimension::Texture3D) {
        rtvDesc.Texture3D = {
            .MipSlice = 0,
            .FirstWSlice = 0,
            .WSize = 1
        };
    }

    m_Device->GetDX12Device()->CreateRenderTargetView(vkTex->GetDX12Resource(), &rtvDesc, m_Allocation.GetCPUHandle(0));
}

void DX12TextureView::CreateDSV() {
    m_Allocation = m_Device->GetDSVAllocator()->Allocate(1);
    m_ClearFlags = ToD3D12ClearFlags(m_Desc.Tex->GetDesc().Format);

    DX12Texture* vkTex = static_cast<DX12Texture*>(m_Desc.Tex);

    D3D12_DEPTH_STENCIL_VIEW_DESC dsvDesc = {
        .Format = ToDXGIFormat(vkTex->GetDesc().Format),
        .ViewDimension = ToD3D12DSVDimension(m_Desc.Dimension)
    };

    if (m_Desc.Dimension == TextureViewDimension::Texture1D) {
        dsvDesc.Texture1D = {
            .MipSlice = 0
        };
    } else if (m_Desc.Dimension == TextureViewDimension::Texture1DArray) {
        dsvDesc.Texture1DArray = {
            .MipSlice = 0,
            .FirstArraySlice = m_Desc.BaseArrayLayer,
            .ArraySize = m_Desc.ArrayLayers
        };
    } else if (m_Desc.Dimension == TextureViewDimension::Texture2D) {
        dsvDesc.Texture2D = {
            .MipSlice = 0,
        };
    } else if (m_Desc.Dimension == TextureViewDimension::Texture2DArray) {
        dsvDesc.Texture2DArray = {
            .MipSlice = 0,
            .FirstArraySlice = m_Desc.BaseArrayLayer,
            .ArraySize = m_Desc.ArrayLayers,
        };
    }

    m_Device->GetDX12Device()->CreateDepthStencilView(vkTex->GetDX12Resource(), &dsvDesc, m_Allocation.GetCPUHandle(0));
}

void DX12TextureView::CreateSRV() {
    m_Allocation = m_Device->GetSRVAllocator()->Allocate(1);

    DX12Texture* vkTex = static_cast<DX12Texture*>(m_Desc.Tex);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {
        .Format = ToDXGIFormat(vkTex->GetDesc().Format),
        .ViewDimension = ToD3D12SRVDimension(m_Desc.Dimension),
        .Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING
    };

    if (m_Desc.Dimension == TextureViewDimension::Texture1D) {
        srvDesc.Texture1D = {
            .MostDetailedMip = 0,
            .MipLevels = 1,
            .ResourceMinLODClamp = 0
        };
    } else if (m_Desc.Dimension == TextureViewDimension::Texture1DArray) {
        srvDesc.Texture1DArray = {
            .MostDetailedMip = 0,
            .MipLevels = 1,
            .FirstArraySlice = m_Desc.BaseArrayLayer,
            .ArraySize = m_Desc.ArrayLayers,
            .ResourceMinLODClamp = 0
        };
    } else if (m_Desc.Dimension == TextureViewDimension::Texture2D) {
        srvDesc.Texture2D = {
            .MostDetailedMip = 0,
            .MipLevels = 1,
            .PlaneSlice = 0,
            .ResourceMinLODClamp = 0
        };
    } else if (m_Desc.Dimension == TextureViewDimension::Texture2DArray) {
        srvDesc.Texture2DArray = {
            .MostDetailedMip = 0,
            .MipLevels = 1,
            .FirstArraySlice = m_Desc.BaseArrayLayer,
            .ArraySize = m_Desc.ArrayLayers,
            .PlaneSlice = 1,
            .ResourceMinLODClamp = 0
        };
    } else if (m_Desc.Dimension == TextureViewDimension::TextureCube) {
        srvDesc.TextureCube = {
            .MostDetailedMip = 0,
            .MipLevels = 1,
            .ResourceMinLODClamp = 0
        };
    } else if (m_Desc.Dimension == TextureViewDimension::TextureCubeArray) {
        srvDesc.TextureCubeArray = {
            .MostDetailedMip = 0,
            .MipLevels = 1,
            .First2DArrayFace = 0,
            .NumCubes = 1,
            .ResourceMinLODClamp = 0
        };
    } else if (m_Desc.Dimension == TextureViewDimension::Texture3D) {
        srvDesc.Texture3D = {
            .MostDetailedMip = 0,
            .MipLevels = 1,
            .ResourceMinLODClamp = 0
        };
    }

    m_Device->GetDX12Device()->CreateShaderResourceView(vkTex->GetDX12Resource(), &srvDesc, m_Allocation.GetCPUHandle(0));
}

DX12Buffer::DX12Buffer(BufferDesc desc, DX12Device* device) {
    m_Desc = desc;
    m_Device = device;

    CreateResource();

    spdlog::info("DX12Buffer Created.");
}

DX12Buffer::~DX12Buffer() {
    if (m_Desc.Usage == BufferUsage::Dynamic) {
        m_Resource->Unmap(0, nullptr);
    }
    m_Device->ReleaseResource(new BufferReleaseResource(m_Resource));

    spdlog::info("DX12Buffer Destroyed.");
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
        .Width = m_Desc.Usage != BufferUsage::Dynamic ? m_Desc.Size : 512 * 512 * 4,
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

    spdlog::info("DX12Sampler Created.");
}

DX12Sampler::~DX12Sampler() {
    spdlog::info("DX12Sampler Destroyed.");
}

SamplerDesc DX12Sampler::GetDesc() {
    return m_Desc;
}

} // dx
