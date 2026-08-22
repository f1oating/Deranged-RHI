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
};

enum BufferBind : uint8_t {
    BUFFER_BIND_VERTEX = 1 << 0,
    BUFFER_BIND_INDEX = 1 << 1,
    BUFFER_BIND_UNIFORM =  1 << 2
};

enum class BufferUsage {
    Default,
    Dynamic,
    Staging
};

struct TextureDesc {
    uint32_t Width;
    uint32_t Height;
    uint32_t MipLevels;
    uint32_t ArrayLayers;
    uint32_t Samples;
    TextureFormat Format;
    TextureType Type;
    uint8_t BindFlags;
};

class TextureView;

class Texture {
public:
    virtual ~Texture() = default;

    virtual TextureView* GetRTV() = 0;

    virtual TextureDesc GetDesc() = 0;

};

struct TextureViewDesc {
    Texture* Tex;
    TextureFormat Format;
};

class TextureView {
public:
    virtual ~TextureView() = default;

    virtual TextureViewDesc GetDesc() = 0;

};

struct BufferDesc {
    uint64_t Size;
    uint8_t BindFlags;
    BufferUsage Usage;
};

class Buffer {
public:
    virtual ~Buffer() = default;

    virtual void* Map() = 0;

    virtual BufferDesc GetDesc() = 0;

};

enum class ResourceLayout {
    Undefined,
    RenderTarget,
    Present
};

struct TextureBarrier {
    Texture* Tex;
    ResourceLayout Layout;
};

#endif //DERANGED_RHI_RESOURCE_H
