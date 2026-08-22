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

class DX12Buffer : public Buffer {
public:
    DX12Buffer(BufferDesc desc, DX12Device* device);
    ~DX12Buffer() override;

    BufferDesc GetDesc() override { return m_Desc; }

private:
    BufferDesc m_Desc;
    DX12Device* m_Device = nullptr;
    ID3D12Resource* m_Resource = nullptr;

};

struct TextureReleaseResource : ReleaseResourceBase {
    ID3D12Resource* Resource;

    TextureReleaseResource(ID3D12Resource* resource)
        : Resource(resource) {}

    void Destroy() override {
        Resource->Release();
    }

};

struct BufferReleaseResource : ReleaseResourceBase {
    ID3D12Resource* Resource;

    BufferReleaseResource(ID3D12Resource* resource)
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
        case TextureFormat::Unknown: return DXGI_FORMAT_UNKNOWN;

        case TextureFormat::R8_UNORM: return DXGI_FORMAT_R8_UNORM;
        case TextureFormat::R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
        case TextureFormat::R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;

        case TextureFormat::R16_UNORM: return DXGI_FORMAT_R16_UNORM;
        case TextureFormat::R16G16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
        case TextureFormat::R16G16B16A16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;

        case TextureFormat::R8_SNORM: return DXGI_FORMAT_R8_SNORM;
        case TextureFormat::R8G8_SNORM: return DXGI_FORMAT_R8G8_SNORM;
        case TextureFormat::R8G8B8A8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;

        case TextureFormat::R16_SNORM: return DXGI_FORMAT_R16_SNORM;
        case TextureFormat::R16G16_SNORM: return DXGI_FORMAT_R16G16_SNORM;
        case TextureFormat::R16G16B16A16_SNORM: return DXGI_FORMAT_R16G16B16A16_SNORM;

        case TextureFormat::R16_FLOAT: return DXGI_FORMAT_R16_FLOAT;
        case TextureFormat::R16G16_FLOAT: return DXGI_FORMAT_R16G16_FLOAT;
        case TextureFormat::R16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;

        case TextureFormat::R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
        case TextureFormat::R32G32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;
        case TextureFormat::R32G32B32_FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
        case TextureFormat::R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;

        case TextureFormat::R8_UINT: return DXGI_FORMAT_R8_UINT;
        case TextureFormat::R8G8_UINT: return DXGI_FORMAT_R8G8_UINT;
        case TextureFormat::R8G8B8A8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;

        case TextureFormat::R16_UINT: return DXGI_FORMAT_R16_UINT;
        case TextureFormat::R16G16_UINT: return DXGI_FORMAT_R16G16_UINT;
        case TextureFormat::R16G16B16A16_UINT: return DXGI_FORMAT_R16G16B16A16_UINT;

        case TextureFormat::R32_UINT: return DXGI_FORMAT_R32_UINT;
        case TextureFormat::R32G32_UINT: return DXGI_FORMAT_R32G32_UINT;
        case TextureFormat::R32G32B32_UINT: return DXGI_FORMAT_R32G32B32_UINT;
        case TextureFormat::R32G32B32A32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;

        case TextureFormat::R8_SINT: return DXGI_FORMAT_R8_SINT;
        case TextureFormat::R8G8_SINT: return DXGI_FORMAT_R8G8_SINT;
        case TextureFormat::R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_SINT;

        case TextureFormat::R16_SINT: return DXGI_FORMAT_R16_SINT;
        case TextureFormat::R16G16_SINT: return DXGI_FORMAT_R16G16_SINT;
        case TextureFormat::R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_SINT;

        case TextureFormat::R32_SINT: return DXGI_FORMAT_R32_SINT;
        case TextureFormat::R32G32_SINT: return DXGI_FORMAT_R32G32_SINT;
        case TextureFormat::R32G32B32_SINT: return DXGI_FORMAT_R32G32B32_SINT;
        case TextureFormat::R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_SINT;

        case TextureFormat::D16_UNORM: return DXGI_FORMAT_D16_UNORM;
        case TextureFormat::D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
        case TextureFormat::D32_SFLOAT_S8_UINT: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
        case TextureFormat::D32_FLOAT: return DXGI_FORMAT_D32_FLOAT;

        default: return DXGI_FORMAT_UNKNOWN;
    }
}

inline TextureFormat FromDXGIFormat(DXGI_FORMAT format) {
    switch (format) {
        case DXGI_FORMAT_UNKNOWN: return TextureFormat::Unknown;

        case DXGI_FORMAT_R8_UNORM: return TextureFormat::R8_UNORM;
        case DXGI_FORMAT_R8G8_UNORM: return TextureFormat::R8G8_UNORM;
        case DXGI_FORMAT_R8G8B8A8_UNORM: return TextureFormat::R8G8B8A8_UNORM;
        case DXGI_FORMAT_B8G8R8A8_UNORM: return TextureFormat::B8G8R8A8_UNORM;

        case DXGI_FORMAT_R16_UNORM: return TextureFormat::R16_UNORM;
        case DXGI_FORMAT_R16G16_UNORM: return TextureFormat::R16G16_UNORM;
        case DXGI_FORMAT_R16G16B16A16_UNORM: return TextureFormat::R16G16B16A16_UNORM;

        case DXGI_FORMAT_R8_SNORM: return TextureFormat::R8_SNORM;
        case DXGI_FORMAT_R8G8_SNORM: return TextureFormat::R8G8_SNORM;
        case DXGI_FORMAT_R8G8B8A8_SNORM: return TextureFormat::R8G8B8A8_SNORM;

        case DXGI_FORMAT_R16_SNORM: return TextureFormat::R16_SNORM;
        case DXGI_FORMAT_R16G16_SNORM: return TextureFormat::R16G16_SNORM;
        case DXGI_FORMAT_R16G16B16A16_SNORM: return TextureFormat::R16G16B16A16_SNORM;

        case DXGI_FORMAT_R16_FLOAT: return TextureFormat::R16_FLOAT;
        case DXGI_FORMAT_R16G16_FLOAT: return TextureFormat::R16G16_FLOAT;
        case DXGI_FORMAT_R16G16B16A16_FLOAT: return TextureFormat::R16G16B16A16_FLOAT;

        case DXGI_FORMAT_R32_FLOAT: return TextureFormat::R32_FLOAT;
        case DXGI_FORMAT_R32G32_FLOAT: return TextureFormat::R32G32_FLOAT;
        case DXGI_FORMAT_R32G32B32_FLOAT: return TextureFormat::R32G32B32_FLOAT;
        case DXGI_FORMAT_R32G32B32A32_FLOAT: return TextureFormat::R32G32B32A32_FLOAT;

        case DXGI_FORMAT_R8_UINT: return TextureFormat::R8_UINT;
        case DXGI_FORMAT_R8G8_UINT: return TextureFormat::R8G8_UINT;
        case DXGI_FORMAT_R8G8B8A8_UINT: return TextureFormat::R8G8B8A8_UINT;

        case DXGI_FORMAT_R16_UINT: return TextureFormat::R16_UINT;
        case DXGI_FORMAT_R16G16_UINT: return TextureFormat::R16G16_UINT;
        case DXGI_FORMAT_R16G16B16A16_UINT: return TextureFormat::R16G16B16A16_UINT;

        case DXGI_FORMAT_R32_UINT: return TextureFormat::R32_UINT;
        case DXGI_FORMAT_R32G32_UINT: return TextureFormat::R32G32_UINT;
        case DXGI_FORMAT_R32G32B32_UINT: return TextureFormat::R32G32B32_UINT;
        case DXGI_FORMAT_R32G32B32A32_UINT: return TextureFormat::R32G32B32A32_UINT;

        case DXGI_FORMAT_R8_SINT: return TextureFormat::R8_SINT;
        case DXGI_FORMAT_R8G8_SINT: return TextureFormat::R8G8_SINT;
        case DXGI_FORMAT_R8G8B8A8_SINT: return TextureFormat::R8G8B8A8_SINT;

        case DXGI_FORMAT_R16_SINT: return TextureFormat::R16_SINT;
        case DXGI_FORMAT_R16G16_SINT: return TextureFormat::R16G16_SINT;
        case DXGI_FORMAT_R16G16B16A16_SINT: return TextureFormat::R16G16B16A16_SINT;

        case DXGI_FORMAT_R32_SINT: return TextureFormat::R32_SINT;
        case DXGI_FORMAT_R32G32_SINT: return TextureFormat::R32G32_SINT;
        case DXGI_FORMAT_R32G32B32_SINT: return TextureFormat::R32G32B32_SINT;
        case DXGI_FORMAT_R32G32B32A32_SINT: return TextureFormat::R32G32B32A32_SINT;

        case DXGI_FORMAT_D16_UNORM: return TextureFormat::D16_UNORM;
        case DXGI_FORMAT_D24_UNORM_S8_UINT: return TextureFormat::D24_UNORM_S8_UINT;
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: return TextureFormat::D32_SFLOAT_S8_UINT;
        case DXGI_FORMAT_D32_FLOAT: return TextureFormat::D32_FLOAT;

        default: return TextureFormat::Unknown;
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

inline D3D12_RESOURCE_FLAGS ToD3D12ResourceFlags(ResourceBindFlags flags) {
    D3D12_RESOURCE_FLAGS dxFlags = D3D12_RESOURCE_FLAG_NONE;

    if (flags & RESOURCE_BIND_RENDER_TARGET) {
        dxFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }
    if (flags & RESOURCE_BIND_DEPTH_STENCIL) {
        dxFlags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    }

    return dxFlags;
}

} // dx

#endif //DERANGED_RHI_DX12RESOURCE_H
