//
// Created by alan on 13/08/2026.
//

#ifndef DERANGED_RHI_DX12RESOURCE_H
#define DERANGED_RHI_DX12RESOURCE_H

#include "Resource.h"
#include <d3d12.h>
#include "ReleaseManager.h"
#include "Backend/DX12/Internal/DescriptorHeapAllocator.h"

namespace dx {

class DX12Device;

class DX12Texture : public Texture {
public:
    DX12Texture(TextureDesc desc, DX12Device* device);
    DX12Texture(TextureDesc desc, ID3D12Resource* res, DX12Device* device);
    ~DX12Texture() override;

    TextureView* GetRTV() override;

    TextureDesc GetDesc() override;

    ID3D12Resource* GetDX12Resource() const { return m_Resource; }
    ResourceLayout GetResourceLayout() const { return m_Layout; }

    void SetResourceLayout(ResourceLayout layout) { m_Layout = layout; }

private:
    TextureDesc m_Desc;
    DX12Device* m_Device = nullptr;
    ID3D12Resource* m_Resource = nullptr;
    ResourceLayout m_Layout = ResourceLayout::Undefined;
    TextureView* m_RTV = nullptr;

};

class DX12TextureView : public TextureView {
public:
    DX12TextureView(TextureViewDesc desc, DX12Device* device);
    ~DX12TextureView() override;

    TextureViewDesc GetDesc() override;

    DescriptorHeapAllocation GetDescriptor() const { return m_Allocation; }

private:
    void CreateRTV();

private:
    TextureViewDesc m_Desc;
    DX12Device* m_Device = nullptr;
    DescriptorHeapAllocation m_Allocation;

};

struct TextureReleaseResource : ReleaseResourceBase {
    ID3D12Resource* Resource;

    TextureReleaseResource(ID3D12Resource* resource)
        : Resource(resource) {}

    void Destroy() override {
        Resource->Release();
    }

};

struct DescriptorAllocationReleaseResource : ReleaseResourceBase {
    DescriptorHeapAllocator* Allocator;
    DescriptorHeapAllocation Allocation;

    DescriptorAllocationReleaseResource(DescriptorHeapAllocator* allocator, DescriptorHeapAllocation allocation)
        : Allocator(allocator), Allocation(allocation) {}

    void Destroy() override {
        Allocator->Free(Allocation);
    }

};

inline DXGI_FORMAT ToDXGIFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::TEXTURE_FORMAT_B8G8R8A8_UNORM:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
        default:
            return DXGI_FORMAT_B8G8R8A8_UNORM;
    }
}

inline TextureFormat FromDXGIFormat(DXGI_FORMAT format) {
    switch (format) {
        case DXGI_FORMAT_B8G8R8A8_UNORM:
            return TextureFormat::TEXTURE_FORMAT_B8G8R8A8_UNORM;
        default:
            return TextureFormat::TEXTURE_FORMAT_B8G8R8A8_UNORM;
    }
}

inline D3D12_RESOURCE_DIMENSION ToD3D12ResourceDimension(TextureType type) {
    switch (type) {
        case TextureType::Texture1D:
            return D3D12_RESOURCE_DIMENSION_TEXTURE1D;
        case TextureType::Texture2D:
            return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
        case TextureType::Texture3D:
            return D3D12_RESOURCE_DIMENSION_TEXTURE3D;
        default:
            return D3D12_RESOURCE_DIMENSION_TEXTURE2D;
    }
}

inline TextureType FromD3D12ResourceDimension(D3D12_RESOURCE_DIMENSION dimension) {
    switch (dimension) {
        case D3D12_RESOURCE_DIMENSION_TEXTURE1D:
            return TextureType::Texture1D;
        case D3D12_RESOURCE_DIMENSION_TEXTURE2D:
            return TextureType::Texture2D;
        case D3D12_RESOURCE_DIMENSION_TEXTURE3D:
            return TextureType::Texture3D;
        default:
            return TextureType::Texture2D;
    }
}

inline D3D12_BARRIER_LAYOUT ToD3D12BarrierLayout(ResourceLayout layout) {
    switch (layout) {
        case ResourceLayout::Undefined:
            return  D3D12_BARRIER_LAYOUT_UNDEFINED;
        case ResourceLayout::RenderTarget:
            return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
        case ResourceLayout::Present:
            return D3D12_BARRIER_LAYOUT_PRESENT;
        default:
            return D3D12_BARRIER_LAYOUT_UNDEFINED;
    }
}

inline ResourceLayout FromD3D12BarrierLayout(D3D12_BARRIER_LAYOUT layout) {
    switch (layout) {
        case D3D12_BARRIER_LAYOUT_UNDEFINED:
            return ResourceLayout::Undefined;
        case D3D12_BARRIER_LAYOUT_RENDER_TARGET:
            return ResourceLayout::RenderTarget;
        case D3D12_BARRIER_LAYOUT_PRESENT:
            return ResourceLayout::Present;
        default:
            return ResourceLayout::Undefined;
    }
}

inline D3D12_BARRIER_ACCESS ToSrcD3D12BarrierAccess(ResourceLayout newLayout) {
    switch (newLayout) {
        case ResourceLayout::RenderTarget:
            return D3D12_BARRIER_ACCESS_COMMON;
        case ResourceLayout::Present:
            return D3D12_BARRIER_ACCESS_RENDER_TARGET;
        default:
            return D3D12_BARRIER_ACCESS_NO_ACCESS;
    }
}

inline D3D12_BARRIER_ACCESS ToDstD3D12BarrierAccess(ResourceLayout newLayout) {
    switch (newLayout) {
        case ResourceLayout::RenderTarget:
            return D3D12_BARRIER_ACCESS_RENDER_TARGET;
        case ResourceLayout::Present:
            return D3D12_BARRIER_ACCESS_COMMON;
        default:
            return D3D12_BARRIER_ACCESS_NO_ACCESS;
    }
}

} // dx

#endif //DERANGED_RHI_DX12RESOURCE_H
