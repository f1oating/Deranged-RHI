//
// Created by alan on 13/08/2026.
//

#ifndef DERANGED_RHI_DX12RESOURCE_H
#define DERANGED_RHI_DX12RESOURCE_H

#include "Resource.h"
#include <d3d12.h>
#include "ReleaseManager.h"
#include "Backend/DX12/Internal/DescriptorHeap.h"

namespace dx {

class DX12Device;
class DX12RenderTargetView;
class DX12DepthStencilView;
class DX12ShaderResourceView;

class DX12Texture : public Texture {
public:
    DX12Texture(TextureDesc desc, DX12Device* device);
    DX12Texture(TextureDesc desc, ID3D12Resource* res, DX12Device* device);
    ~DX12Texture() override;

    RenderTargetView* GetRTV() override;
    DepthStencilView* GetDSV() override;
    ShaderResourceView* GetSRV() override;

    TextureDesc GetDesc() override;

    ID3D12Resource* GetDX12Resource() const { return m_Resource; }
    ImageLayout GetResourceLayout() const { return m_Layout; }

    void SetResourceLayout(ImageLayout layout) { m_Layout = layout; }

private:
    DX12Device* m_Device = nullptr;

    TextureDesc m_Desc;

    ID3D12Resource* m_Resource = nullptr;

    ImageLayout m_Layout = ImageLayout::Undefined;

    DX12RenderTargetView* m_RTV = nullptr;
    DX12DepthStencilView* m_DSV = nullptr;
    DX12ShaderResourceView* m_SRV = nullptr;

};

class DX12RenderTargetView : public RenderTargetView {
public:
    DX12RenderTargetView(DX12Texture* texture, DX12Device* device);
    ~DX12RenderTargetView();

    DescriptorHeapAllocation GetAllocation() const { return m_Allocation; }

private:
    DX12Device* m_Device = nullptr;

    DX12Texture* m_Texture = nullptr;

    DescriptorHeapAllocation m_Allocation;

};

class DX12DepthStencilView : public DepthStencilView {
public:
    DX12DepthStencilView(DX12Texture* texture, DX12Device* device);
    ~DX12DepthStencilView();

    DescriptorHeapAllocation GetAllocation() const { return m_Allocation; }
    D3D12_CLEAR_FLAGS GetDX12ClearFlags() const { return m_ClearFlags; }

private:
    DX12Device* m_Device = nullptr;
    DX12Texture* m_Texture = nullptr;

    DescriptorHeapAllocation m_Allocation;

    D3D12_CLEAR_FLAGS m_ClearFlags = D3D12_CLEAR_FLAG_DEPTH;

};

class DX12ShaderResourceView : public ShaderResourceView {
public:
    DX12ShaderResourceView(DX12Texture* texture, DX12Device* device);
    ~DX12ShaderResourceView();

    DX12Texture* GetDXTexture() const { return m_Texture; }
    D3D12_SHADER_RESOURCE_VIEW_DESC GetDXView() const { return m_View; }

private:
    DX12Device* m_Device = nullptr;
    DX12Texture* m_Texture = nullptr;

    D3D12_SHADER_RESOURCE_VIEW_DESC m_View;

};

class DX12Buffer : public Buffer {
public:
    DX12Buffer(BufferDesc desc, DX12Device* device);
    ~DX12Buffer() override;

    void* Map() override;

    BufferDesc GetDesc() override;
    ID3D12Resource* GetDX12Resource() const { return m_Resource; }
    uint64_t GetOffset() const { return m_Offset; }

private:
    void CreateResource();

private:
    DX12Device* m_Device = nullptr;

    BufferDesc m_Desc;

    ID3D12Resource* m_Resource = nullptr;

    uint64_t m_Offset = 0;
    void* m_Mapped = nullptr;

};

class DX12Sampler : public Sampler {
public:
    DX12Sampler(SamplerDesc desc, DX12Device* device);
    ~DX12Sampler();

    SamplerDesc GetDesc() override;

    D3D12_SAMPLER_DESC GetDXSampler() const { return m_Sampler; }

private:
    DX12Device* m_Device = nullptr;

    SamplerDesc m_Desc;

    D3D12_SAMPLER_DESC m_Sampler;

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
    DescriptorHeap* Allocator;
    DescriptorHeapAllocation Allocation;

    DescriptorAllocationReleaseResource(DescriptorHeap* allocator, DescriptorHeapAllocation allocation)
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

inline D3D12_COMPARISON_FUNC ToD3D12ComparisonFunc(CompareOp op) {
    switch (op) {
        case CompareOp::Never:
            return D3D12_COMPARISON_FUNC_NEVER;
        case CompareOp::Less:
            return D3D12_COMPARISON_FUNC_LESS;
        case CompareOp::Equal:
            return D3D12_COMPARISON_FUNC_EQUAL;
        case CompareOp::LessOrEqual:
            return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case CompareOp::Greater:
            return D3D12_COMPARISON_FUNC_GREATER;
        case CompareOp::NotEqual:
            return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case CompareOp::GreaterOrEqual:
            return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case CompareOp::Always:
            return D3D12_COMPARISON_FUNC_ALWAYS;
        default:
            return D3D12_COMPARISON_FUNC_NEVER;
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

inline D3D12_BARRIER_LAYOUT ToD3D12BarrierLayout(ImageLayout layout) {
    switch (layout) {
        case ImageLayout::Undefined:
            return  D3D12_BARRIER_LAYOUT_UNDEFINED;
        case ImageLayout::TransferSRC:
            return  D3D12_BARRIER_LAYOUT_COPY_SOURCE;
        case ImageLayout::TransferDST:
            return  D3D12_BARRIER_LAYOUT_COPY_DEST;
        case ImageLayout::ShaderResource:
            return  D3D12_BARRIER_LAYOUT_SHADER_RESOURCE;
        case ImageLayout::DepthStencil:
            return D3D12_BARRIER_LAYOUT_DEPTH_STENCIL_WRITE;
        case ImageLayout::RenderTarget:
            return D3D12_BARRIER_LAYOUT_RENDER_TARGET;
        case ImageLayout::Present:
            return D3D12_BARRIER_LAYOUT_PRESENT;
        default:
            return D3D12_BARRIER_LAYOUT_UNDEFINED;
    }
}

inline D3D12_BARRIER_ACCESS ToD3D12BarrierAccess(uint32_t flags) {
    D3D12_BARRIER_ACCESS dxFlags = D3D12_BARRIER_ACCESS_COMMON;

    if (flags & ACCESS_NONE) {
        dxFlags |= D3D12_BARRIER_ACCESS_NO_ACCESS;
    }
    if (flags & ACCESS_SHADER_READ) {
        dxFlags |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
    }
    if (flags & ACCESS_SHADER_WRITE) {
        dxFlags |= D3D12_BARRIER_ACCESS_SHADER_RESOURCE;
    }
    if (flags & ACCESS_COLOR_ATTACHMENT_READ) {
        dxFlags |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
    }
    if (flags & ACCESS_COLOR_ATTACHMENT_WRITE) {
        dxFlags |= D3D12_BARRIER_ACCESS_RENDER_TARGET;
    }
    if (flags & ACCESS_DEPTH_STENCIL_ATTACHMENT_READ) {
        dxFlags |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_READ;
    }
    if (flags & ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE) {
        dxFlags |= D3D12_BARRIER_ACCESS_DEPTH_STENCIL_WRITE;
    }
    if (flags & ACCESS_TRANSFER_READ) {
        dxFlags |= D3D12_BARRIER_ACCESS_COPY_SOURCE;
    }
    if (flags & ACCESS_TRANSFER_WRITE) {
        dxFlags = D3D12_BARRIER_ACCESS_COPY_DEST;
    }
    if (flags & ACCESS_VERTEX_READ) {
        dxFlags |= D3D12_BARRIER_ACCESS_VERTEX_BUFFER;
    }
    if (flags & ACCESS_INDEX_READ) {
        dxFlags |= D3D12_BARRIER_ACCESS_INDEX_BUFFER;
    }
    if (flags & ACCESS_UNIFORM_READ) {
        dxFlags |= D3D12_BARRIER_ACCESS_CONSTANT_BUFFER;
    }

    return dxFlags;
}

inline D3D12_BARRIER_SYNC ToD3D12BarrierSync(uint32_t flags) {
    D3D12_BARRIER_SYNC dxFlags = D3D12_BARRIER_SYNC_NONE;

    if (flags & PIPELINE_STAGE_NONE) {
        dxFlags |= D3D12_BARRIER_SYNC_NONE;
    }
    if (flags & PIPELINE_STAGE_INDEX_INPUT) {
        dxFlags |= D3D12_BARRIER_SYNC_INDEX_INPUT;
    }
    if (flags & PIPELINE_STAGE_VERTEX_INPUT) {
        dxFlags |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
    }
    if (flags & PIPELINE_STAGE_VERTEX_SHADER) {
        dxFlags |= D3D12_BARRIER_SYNC_VERTEX_SHADING;
    }
    if (flags & PIPELINE_STAGE_FRAGMENT_SHADER) {
        dxFlags |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
    }
    if (flags & PIPELINE_STAGE_EARLY_FRAGMENT_TESTS) {
        dxFlags |= D3D12_BARRIER_SYNC_PIXEL_SHADING;
    }
    if (flags & PIPELINE_STAGE_LATE_FRAGMENT_TESTS) {
        dxFlags |= D3D12_BARRIER_SYNC_DEPTH_STENCIL;
    }
    if (flags & PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT) {
        dxFlags |= D3D12_BARRIER_SYNC_RENDER_TARGET;
    }
    if (flags & PIPELINE_STAGE_COMPUTE_SHADER) {
        dxFlags |= D3D12_BARRIER_SYNC_COMPUTE_SHADING;
    }
    if (flags & PIPELINE_STAGE_TRANSFER) {
        dxFlags |= D3D12_BARRIER_SYNC_COPY;
    }
    if (flags & PIPELINE_STAGE_ALL_GRAPHICS) {
        dxFlags |= D3D12_BARRIER_SYNC_ALL_SHADING;
    }
    if (flags & PIPELINE_STAGE_ALL_COMMANDS) {
        dxFlags |= D3D12_BARRIER_SYNC_ALL;
    }

    return dxFlags;
}

inline D3D12_RESOURCE_FLAGS ToD3D12TexResourceFlags(uint8_t flags) {
    D3D12_RESOURCE_FLAGS dxFlags = D3D12_RESOURCE_FLAG_NONE;

    if (flags & TEXTURE_BIND_RENDER_TARGET) {
        dxFlags |= D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
    }
    if (flags & TEXTURE_BIND_DEPTH_STENCIL) {
        dxFlags |= D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
    }

    return dxFlags;
}

inline D3D12_RESOURCE_FLAGS ToD3D12BufResourceFlags(uint8_t flags) {
    D3D12_RESOURCE_FLAGS dxFlags = D3D12_RESOURCE_FLAG_NONE;

    return dxFlags;
}

inline D3D12_FILTER ToD3D12Filter(Filter filter) {
    switch (filter) {
        case Filter::Nearest:
            return D3D12_FILTER_MIN_MAG_MIP_POINT;
        case Filter::Linear:
            return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        default:
            return D3D12_FILTER_MIN_MAG_MIP_POINT;
    }
}

inline D3D12_TEXTURE_ADDRESS_MODE ToD3D12TextureAddressMode(AddressMode mode) {
    switch (mode) {
        case AddressMode::Repeat:
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case AddressMode::MirroredRepeat:
            return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        default:
            return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    }
}

inline D3D12_CLEAR_FLAGS ToD3D12ClearFlags(TextureFormat format) {
    switch (format) {
        case TextureFormat::D16_UNORM:
        case TextureFormat::D32_FLOAT:
            return D3D12_CLEAR_FLAG_DEPTH;
        case TextureFormat::D24_UNORM_S8_UINT:
        case TextureFormat::D32_SFLOAT_S8_UINT:
            return D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL;
        default:
            return D3D12_CLEAR_FLAG_DEPTH;
    }
}

inline uint32_t GetFormatSize(TextureFormat format) {
    switch (format) {
        case TextureFormat::Unknown: return 0;

        case TextureFormat::R8_UNORM: return 1;
        case TextureFormat::R8G8_UNORM: return 2;
        case TextureFormat::R8G8B8A8_UNORM: return 3;
        case TextureFormat::B8G8R8A8_UNORM: return 4;

        case TextureFormat::R16_UNORM: return 2;
        case TextureFormat::R16G16_UNORM: return 4;
        case TextureFormat::R16G16B16A16_UNORM: return 8;

        case TextureFormat::R8_SNORM: return 1;
        case TextureFormat::R8G8_SNORM: return 2;
        case TextureFormat::R8G8B8A8_SNORM: return 4;

        case TextureFormat::R16_SNORM: return 2;
        case TextureFormat::R16G16_SNORM: return 4;
        case TextureFormat::R16G16B16A16_SNORM: return 8;

        case TextureFormat::R16_FLOAT: return 2;
        case TextureFormat::R16G16_FLOAT: return 4;
        case TextureFormat::R16G16B16A16_FLOAT: return 8;

        case TextureFormat::R32_FLOAT: return 4;
        case TextureFormat::R32G32_FLOAT: return 8;
        case TextureFormat::R32G32B32_FLOAT: return 12;
        case TextureFormat::R32G32B32A32_FLOAT: return 16;

        case TextureFormat::R8_UINT: return 1;
        case TextureFormat::R8G8_UINT: return 2;
        case TextureFormat::R8G8B8A8_UINT: return 4;

        case TextureFormat::R16_UINT: return 2;
        case TextureFormat::R16G16_UINT: return 4;
        case TextureFormat::R16G16B16A16_UINT: return 8;

        case TextureFormat::R32_UINT: return 4;
        case TextureFormat::R32G32_UINT: return 8;
        case TextureFormat::R32G32B32_UINT: return 12;
        case TextureFormat::R32G32B32A32_UINT: return 16;

        case TextureFormat::R8_SINT: return 1;
        case TextureFormat::R8G8_SINT: return 2;
        case TextureFormat::R8G8B8A8_SINT: return 4;

        case TextureFormat::R16_SINT: return 2;
        case TextureFormat::R16G16_SINT: return 4;
        case TextureFormat::R16G16B16A16_SINT: return 8;

        case TextureFormat::R32_SINT: return 4;
        case TextureFormat::R32G32_SINT: return 8;
        case TextureFormat::R32G32B32_SINT: return 12;
        case TextureFormat::R32G32B32A32_SINT: return 16;

        case TextureFormat::D16_UNORM: return 2;
        case TextureFormat::D24_UNORM_S8_UINT: return 4;
        case TextureFormat::D32_SFLOAT_S8_UINT: return 8;
        case TextureFormat::D32_FLOAT: return 4;

        default: return 0;
    }
}

} // dx

#endif //DERANGED_RHI_DX12RESOURCE_H
