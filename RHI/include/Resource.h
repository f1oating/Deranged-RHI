//
// Created by alan on 13/08/2026.
//

#ifndef DERANGED_RHI_RESOURCE_H
#define DERANGED_RHI_RESOURCE_H

#include <cstdint>

enum class TextureFormat {
    Unknown,

    R8_UNORM, R8G8_UNORM, R8G8B8A8_UNORM, B8G8R8A8_UNORM,
    R16_UNORM, R16G16_UNORM, R16G16B16A16_UNORM,

    R8_SNORM, R8G8_SNORM, R8G8B8A8_SNORM, B8G8R8A8_SNORM,
    R16_SNORM, R16G16_SNORM, R16G16B16A16_SNORM,

    R16_FLOAT, R16G16_FLOAT, R16G16B16A16_FLOAT,
    R32_FLOAT, R32G32_FLOAT, R32G32B32_FLOAT, R32G32B32A32_FLOAT,

    R8_UINT, R8G8_UINT, R8G8B8A8_UINT, B8G8R8A8_UINT,
    R16_UINT, R16G16_UINT, R16G16B16A16_UINT,
    R32_UINT, R32G32_UINT, R32G32B32_UINT, R32G32B32A32_UINT,

    R8_SINT, R8G8_SINT, R8G8B8A8_SINT, B8G8R8A8_SINT,
    R16_SINT, R16G16_SINT, R16G16B16A16_SINT,
    R32_SINT, R32G32_SINT, R32G32B32_SINT, R32G32B32A32_SINT,

    D16_UNORM, D24_UNORM_S8_UINT,
    D32_SFLOAT_S8_UINT, D32_FLOAT
};

enum class TextureType {
    Texture1D, Texture2D, Texture3D
};

enum TextureBind : uint8_t {
    TEXTURE_BIND_SHADER_RESOURCE = 1 << 0,
    TEXTURE_BIND_RENDER_TARGET = 1 << 1,
    TEXTURE_BIND_DEPTH_STENCIL = 1 << 2,
    TEXTURE_BIND_TRANSFER_SRC = 1 << 3,
    TEXTURE_BIND_TRANSFER_DST = 1 << 4,
};

enum BufferBind : uint8_t {
    BUFFER_BIND_VERTEX = 1 << 0,
    BUFFER_BIND_INDEX = 1 << 1,
    BUFFER_BIND_UNIFORM =  1 << 2,
    BUFFER_BIND_TRANSFER_SRC = 1 << 3,
    BUFFER_BIND_TRANSFER_DST = 1 << 4,
};

enum class BufferUsage {
    Default,
    Dynamic,
    Staging
};

struct TextureDesc {
    uint32_t Width = 1;
    uint32_t Height = 1;
    uint32_t MipLevels = 1;
    uint32_t ArrayLayers = 1;
    uint32_t Samples = 1;
    TextureFormat Format = TextureFormat::R8G8B8A8_SNORM;
    TextureType Type = TextureType::Texture2D;
    uint8_t BindFlags = TEXTURE_BIND_SHADER_RESOURCE;
};

class RenderTargetView;
class DepthStencilView;
class ShaderResourceView;

class Texture {
public:
    virtual ~Texture() = default;

    virtual RenderTargetView* GetRTV() = 0;
    virtual DepthStencilView* GetDSV() = 0;
    virtual ShaderResourceView* GetSRV() = 0;

    virtual TextureDesc GetDesc() = 0;

};

class RenderTargetView {
public:
    virtual ~RenderTargetView() = default;

};

class DepthStencilView {
public:
    virtual ~DepthStencilView() = default;

};

class ShaderResourceView {
public:
    virtual ~ShaderResourceView() = default;

};

struct BufferDesc {
    uint64_t Size = 1;
    uint8_t BindFlags = 0;
    uint32_t Stride = 1;
    BufferUsage Usage = BufferUsage::Default;
};

class Buffer {
public:
    virtual ~Buffer() = default;

    virtual void* Map() = 0;

    virtual BufferDesc GetDesc() = 0;

};

enum class ImageLayout {
    Undefined,
    TransferSRC,
    TransferDST,
    ShaderResource,
    RenderTarget,
    Present
};

enum AccessFlags : uint32_t {
    ACCESS_NONE = 1 << 0,
    ACCESS_SHADER_READ = 1 << 1,
    ACCESS_SHADER_WRITE = 1 << 2,
    ACCESS_COLOR_ATTACHMENT_READ = 1 << 3,
    ACCESS_COLOR_ATTACHMENT_WRITE = 1 << 4,
    ACCESS_DEPTH_STENCIL_ATTACHMENT_READ = 1 << 5,
    ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE = 1 << 6,
    ACCESS_TRANSFER_READ = 1 << 7,
    ACCESS_TRANSFER_WRITE = 1 << 8,
    ACCESS_VERTEX_READ = 1 << 9,
    ACCESS_INDEX_READ = 1 << 10,
    ACCESS_UNIFORM_READ = 1 << 11
};

enum PipelineStageFlags : uint32_t {
    PIPELINE_STAGE_NONE = 1 << 0,
    PIPELINE_STAGE_VERTEX_INPUT = 1 << 1,
    PIPELINE_STAGE_VERTEX_SHADER = 1 << 2,
    PIPELINE_STAGE_FRAGMENT_SHADER = 1 << 3,
    PIPELINE_STAGE_EARLY_FRAGMENT_TESTS = 1 << 4,
    PIPELINE_STAGE_LATE_FRAGMENT_TESTS = 1 << 5,
    PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT = 1 << 6,
    PIPELINE_STAGE_COMPUTE_SHADER = 1 << 7,
    PIPELINE_STAGE_TRANSFER = 1 << 8,
    PIPELINE_STAGE_ALL_GRAPHICS = 1 << 10,
    PIPELINE_STAGE_ALL_COMMANDS = 1 << 11
};

struct TextureBarrier {
    Texture* Tex;
    ImageLayout Layout;
    uint32_t SrcAccessFlags;
    uint32_t DstAccessFlags;
};

struct BufferBarrier {
    Buffer* Buf;
    uint32_t SrcAccessFlags;
    uint32_t DstAccessFlags;
};

enum class CompareOp {
    Never,
    Less,
    Equal,
    LessOrEqual,
    Greater,
    NotEqual,
    GreaterOrEqual,
    Always
};

enum class AddressMode {
    Repeat,
    MirroredRepeat
};

enum class Filter {
    Nearest,
    Linear
};

struct SamplerDesc {
    Filter Filtering = Filter::Nearest;
    CompareOp Compare = CompareOp::Never;
    AddressMode AddressU = AddressMode::Repeat;
    AddressMode AddressV = AddressMode::Repeat;
    AddressMode AddressW = AddressMode::Repeat;
    float MipLodBias = 0.0f;
    uint32_t MaxAnisotropy = 0;
    float MinLod = 0.0f;
    float MaxLod = 1.0f;
};

class Sampler {
public:
    virtual ~Sampler() = default;

    virtual SamplerDesc GetDesc() = 0;

};

#endif //DERANGED_RHI_RESOURCE_H
