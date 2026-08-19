//
// Created by alan on 13/08/2026.
//

#ifndef DERANGED_RHI_RESOURCE_H
#define DERANGED_RHI_RESOURCE_H

#include <cstdint>

enum class TextureFormat {
    TEXTURE_FORMAT_B8G8R8A8_UNORM
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

};

struct TextureViewDesc {
    Texture* Tex;
    TextureFormat Format;
};

class TextureView {
public:
    virtual ~TextureView() = default;

};

enum class ResourceLayout {
    RenderTarget,
    Present
};

struct TextureBarrier {
    Texture* Tex;
    ResourceLayout Layout;
};

#endif //DERANGED_RHI_RESOURCE_H
