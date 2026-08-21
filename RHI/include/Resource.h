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

struct TextureDesc {
    uint32_t Width;
    uint32_t Height;
    uint32_t MipLevels;
    uint32_t ArrayLayers;
    uint32_t Samples;
    TextureFormat Format;
    TextureType Type;
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
