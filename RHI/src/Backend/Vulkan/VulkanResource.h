//
// Created by alan on 13/08/2026.
//

#ifndef DERANGED_RHI_VULKANRESOURCE_H
#define DERANGED_RHI_VULKANRESOURCE_H

#include "Resource.h"
#include <volk.h>
#include "ReleaseManager.h"

namespace vk {

class VulkanDevice;

class VulkanTexture : public Texture {
public:
    VulkanTexture(TextureDesc desc, VulkanDevice* device);
    VulkanTexture(TextureDesc desc, VulkanDevice* device, VkImage image);
    ~VulkanTexture() override;

    TextureView* GetRTV() override;

    TextureDesc GetDesc() override;

    VkImage GetVkImage() const { return m_Image; }
    ImageLayout GetLayout() const { return m_Layout; }

    void SetLayout(ImageLayout layout) { m_Layout = layout; }

private:
    void CreateTexture();
    void CreateMemory();

private:
    VulkanDevice* m_Device = nullptr;
    TextureDesc m_Desc;
    VkImage m_Image = nullptr;
    VkDeviceMemory m_Memory = nullptr;
    TextureView* m_RTV = nullptr;
    ImageLayout m_Layout = ImageLayout::Undefined;
    uint64_t m_SizeInBytes = 0;

};

class VulkanTextureView : public TextureView {
public:
    VulkanTextureView(TextureViewDesc desc, VulkanDevice* device);
    ~VulkanTextureView() override;

    TextureViewDesc GetDesc() override;

    VkImageView GetVkImageView() const { return m_ImageView; }
    VulkanTexture* GetTexture() const { return m_Texture; }

private:
    VulkanDevice* m_Device = nullptr;
    TextureViewDesc m_Desc;
    VkImageView m_ImageView = nullptr;
    VulkanTexture* m_Texture = nullptr;

};

class VulkanBuffer : public Buffer {
public:
    VulkanBuffer(BufferDesc desc, VulkanDevice* device);
    ~VulkanBuffer() override;

    void* Map() override;

    BufferDesc GetDesc() override;
    VkBuffer GetVkBuffer() const { return m_Buffer; }

private:
    void CreateBuffer();
    void CreateMemory();

private:
    VulkanDevice* m_Device = nullptr;
    BufferDesc m_Desc;
    VkBuffer m_Buffer = nullptr;
    VkDeviceMemory m_Memory = nullptr;
    uint64_t m_SizeInBytes = 0;
    uint64_t m_Offset = 0;
    void* m_Mapped = nullptr;

};

struct ImageReleaseResource : ReleaseResourceBase {
    VkDevice Device;
    VkImage Image;
    VkDeviceMemory Memory;

    ImageReleaseResource(VkDevice device, VkImage image, VkDeviceMemory memory)
        : Device(device), Image(image), Memory(memory) {}

    void Destroy() override {
        vkFreeMemory(Device, Memory, nullptr);
        vkDestroyImage(Device, Image, nullptr);
    }

};

struct ImageViewReleaseResource : ReleaseResourceBase {
    VkDevice Device;
    VkImageView ImageView;

    ImageViewReleaseResource(VkDevice device, VkImageView imageView)
        : Device(device), ImageView(imageView) {}

    void Destroy() override {
        vkDestroyImageView(Device, ImageView, nullptr);
    }

};

struct BufferReleaseResource : ReleaseResourceBase {
    VkDevice Device;
    VkBuffer Buffer;
    VkDeviceMemory Memory;

    BufferReleaseResource(VkDevice device, VkBuffer buffer, VkDeviceMemory memory)
        : Device(device), Buffer(buffer), Memory(memory) {}

    void Destroy() override {
        if (Memory) {
            vkFreeMemory(Device, Memory, nullptr);
        }
        vkDestroyBuffer(Device, Buffer, nullptr);
    }

};

inline VkFormat ToVkFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::Unknown: return VK_FORMAT_UNDEFINED;

        case TextureFormat::R8_UNORM: return VK_FORMAT_R8_UNORM;
        case TextureFormat::R8G8_UNORM: return VK_FORMAT_R8G8_UNORM;
        case TextureFormat::R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
        case TextureFormat::B8G8R8A8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;

        case TextureFormat::R16_UNORM: return VK_FORMAT_R16_UNORM;
        case TextureFormat::R16G16_UNORM: return VK_FORMAT_R16G16_UNORM;
        case TextureFormat::R16G16B16A16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;

        case TextureFormat::R8_SNORM: return VK_FORMAT_R8_SNORM;
        case TextureFormat::R8G8_SNORM: return VK_FORMAT_R8G8_SNORM;
        case TextureFormat::R8G8B8A8_SNORM: return VK_FORMAT_R8G8B8A8_SNORM;

        case TextureFormat::R16_SNORM: return VK_FORMAT_R16_SNORM;
        case TextureFormat::R16G16_SNORM: return VK_FORMAT_R16G16_SNORM;
        case TextureFormat::R16G16B16A16_SNORM: return VK_FORMAT_R16G16B16A16_SNORM;

        case TextureFormat::R16_FLOAT: return VK_FORMAT_R16_SFLOAT;
        case TextureFormat::R16G16_FLOAT: return VK_FORMAT_R16G16_SFLOAT;
        case TextureFormat::R16G16B16A16_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;

        case TextureFormat::R32_FLOAT: return VK_FORMAT_R32_SFLOAT;
        case TextureFormat::R32G32_FLOAT: return VK_FORMAT_R32G32_SFLOAT;
        case TextureFormat::R32G32B32_FLOAT: return VK_FORMAT_R32G32B32_SFLOAT;
        case TextureFormat::R32G32B32A32_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;

        case TextureFormat::R8_UINT: return VK_FORMAT_R8_UINT;
        case TextureFormat::R8G8_UINT: return VK_FORMAT_R8G8_UINT;
        case TextureFormat::R8G8B8A8_UINT: return VK_FORMAT_R8G8B8A8_UINT;

        case TextureFormat::R16_UINT: return VK_FORMAT_R16_UINT;
        case TextureFormat::R16G16_UINT: return VK_FORMAT_R16G16_UINT;
        case TextureFormat::R16G16B16A16_UINT: return VK_FORMAT_R16G16B16A16_UINT;

        case TextureFormat::R32_UINT: return VK_FORMAT_R32_UINT;
        case TextureFormat::R32G32_UINT: return VK_FORMAT_R32G32_UINT;
        case TextureFormat::R32G32B32_UINT: return VK_FORMAT_R32G32B32_UINT;
        case TextureFormat::R32G32B32A32_UINT: return VK_FORMAT_R32G32B32A32_UINT;

        case TextureFormat::R8_SINT: return VK_FORMAT_R8_SINT;
        case TextureFormat::R8G8_SINT: return VK_FORMAT_R8G8_SINT;
        case TextureFormat::R8G8B8A8_SINT: return VK_FORMAT_R8G8B8A8_SINT;

        case TextureFormat::R16_SINT: return VK_FORMAT_R16_SINT;
        case TextureFormat::R16G16_SINT: return VK_FORMAT_R16G16_SINT;
        case TextureFormat::R16G16B16A16_SINT: return VK_FORMAT_R16G16B16A16_SINT;

        case TextureFormat::R32_SINT: return VK_FORMAT_R32_SINT;
        case TextureFormat::R32G32_SINT: return VK_FORMAT_R32G32_SINT;
        case TextureFormat::R32G32B32_SINT: return VK_FORMAT_R32G32B32_SINT;
        case TextureFormat::R32G32B32A32_SINT: return VK_FORMAT_R32G32B32A32_SINT;

        case TextureFormat::D16_UNORM: return VK_FORMAT_D16_UNORM;
        case TextureFormat::D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
        case TextureFormat::D32_SFLOAT_S8_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
        case TextureFormat::D32_FLOAT: return VK_FORMAT_D32_SFLOAT;

        default: return VK_FORMAT_UNDEFINED;
    }
}

inline TextureFormat FromVkFormat(VkFormat format) {
    switch (format) {
        case VK_FORMAT_UNDEFINED: return TextureFormat::Unknown;

        case VK_FORMAT_R8_UNORM: return TextureFormat::R8_UNORM;
        case VK_FORMAT_R8G8_UNORM: return TextureFormat::R8G8_UNORM;
        case VK_FORMAT_R8G8B8A8_UNORM: return TextureFormat::R8G8B8A8_UNORM;
        case VK_FORMAT_B8G8R8A8_UNORM: return TextureFormat::B8G8R8A8_UNORM;

        case VK_FORMAT_R16_UNORM: return TextureFormat::R16_UNORM;
        case VK_FORMAT_R16G16_UNORM: return TextureFormat::R16G16_UNORM;
        case VK_FORMAT_R16G16B16A16_UNORM: return TextureFormat::R16G16B16A16_UNORM;

        case VK_FORMAT_R8_SNORM: return TextureFormat::R8_SNORM;
        case VK_FORMAT_R8G8_SNORM: return TextureFormat::R8G8_SNORM;
        case VK_FORMAT_R8G8B8A8_SNORM: return TextureFormat::R8G8B8A8_SNORM;

        case VK_FORMAT_R16_SNORM: return TextureFormat::R16_SNORM;
        case VK_FORMAT_R16G16_SNORM: return TextureFormat::R16G16_SNORM;
        case VK_FORMAT_R16G16B16A16_SNORM: return TextureFormat::R16G16B16A16_SNORM;

        case VK_FORMAT_R16_SFLOAT: return TextureFormat::R16_FLOAT;
        case VK_FORMAT_R16G16_SFLOAT: return TextureFormat::R16G16_FLOAT;
        case VK_FORMAT_R16G16B16A16_SFLOAT: return TextureFormat::R16G16B16A16_FLOAT;

        case VK_FORMAT_R32_SFLOAT: return TextureFormat::R32_FLOAT;
        case VK_FORMAT_R32G32_SFLOAT: return TextureFormat::R32G32_FLOAT;
        case VK_FORMAT_R32G32B32_SFLOAT: return TextureFormat::R32G32B32_FLOAT;
        case VK_FORMAT_R32G32B32A32_SFLOAT: return TextureFormat::R32G32B32A32_FLOAT;

        case VK_FORMAT_R8_UINT: return TextureFormat::R8_UINT;
        case VK_FORMAT_R8G8_UINT: return TextureFormat::R8G8_UINT;
        case VK_FORMAT_R8G8B8A8_UINT: return TextureFormat::R8G8B8A8_UINT;

        case VK_FORMAT_R16_UINT: return TextureFormat::R16_UINT;
        case VK_FORMAT_R16G16_UINT: return TextureFormat::R16G16_UINT;
        case VK_FORMAT_R16G16B16A16_UINT: return TextureFormat::R16G16B16A16_UINT;

        case VK_FORMAT_R32_UINT: return TextureFormat::R32_UINT;
        case VK_FORMAT_R32G32_UINT: return TextureFormat::R32G32_UINT;
        case VK_FORMAT_R32G32B32_UINT: return TextureFormat::R32G32B32_UINT;
        case VK_FORMAT_R32G32B32A32_UINT: return TextureFormat::R32G32B32A32_UINT;

        case VK_FORMAT_R8_SINT: return TextureFormat::R8_SINT;
        case VK_FORMAT_R8G8_SINT: return TextureFormat::R8G8_SINT;
        case VK_FORMAT_R8G8B8A8_SINT: return TextureFormat::R8G8B8A8_SINT;

        case VK_FORMAT_R16_SINT: return TextureFormat::R16_SINT;
        case VK_FORMAT_R16G16_SINT: return TextureFormat::R16G16_SINT;
        case VK_FORMAT_R16G16B16A16_SINT: return TextureFormat::R16G16B16A16_SINT;

        case VK_FORMAT_R32_SINT: return TextureFormat::R32_SINT;
        case VK_FORMAT_R32G32_SINT: return TextureFormat::R32G32_SINT;
        case VK_FORMAT_R32G32B32_SINT: return TextureFormat::R32G32B32_SINT;
        case VK_FORMAT_R32G32B32A32_SINT: return TextureFormat::R32G32B32A32_SINT;

        case VK_FORMAT_D16_UNORM: return TextureFormat::D16_UNORM;
        case VK_FORMAT_D24_UNORM_S8_UINT: return TextureFormat::D24_UNORM_S8_UINT;
        case VK_FORMAT_D32_SFLOAT_S8_UINT: return TextureFormat::D32_SFLOAT_S8_UINT;
        case VK_FORMAT_D32_SFLOAT: return TextureFormat::D32_FLOAT;

        default: return TextureFormat::Unknown;
    }
}

inline VkImageType ToVkImageType(TextureType type) {
    switch (type) {
        case TextureType::Texture1D:
            return VK_IMAGE_TYPE_1D;
        case TextureType::Texture2D:
            return VK_IMAGE_TYPE_2D;
        case TextureType::Texture3D:
            return VK_IMAGE_TYPE_3D;
        default:
            return VK_IMAGE_TYPE_2D;
    }
}

inline TextureType FromVkImageType(VkImageType type) {
    switch (type) {
        case VK_IMAGE_TYPE_1D:
            return TextureType::Texture1D;
        case VK_IMAGE_TYPE_2D:
            return TextureType::Texture2D;
        case VK_IMAGE_TYPE_3D:
            return TextureType::Texture3D;
        default:
            return TextureType::Texture2D;
    }
}

inline VkSampleCountFlagBits ToVkSampleCountFlagBits(uint32_t sampleCount) {
    switch (sampleCount) {
        case 1:
            return VK_SAMPLE_COUNT_1_BIT;
        case 2:
            return VK_SAMPLE_COUNT_2_BIT;
        case 4:
            return VK_SAMPLE_COUNT_4_BIT;
        case 8:
            return VK_SAMPLE_COUNT_8_BIT;
        case 16:
            return VK_SAMPLE_COUNT_16_BIT;
        case 32:
            return VK_SAMPLE_COUNT_32_BIT;
        case 64:
            return VK_SAMPLE_COUNT_64_BIT;
        default:
            return VK_SAMPLE_COUNT_1_BIT;
    }
}

inline VkImageLayout ToVkImageLayout(ImageLayout layout) {
    switch (layout) {
        case ImageLayout::Undefined:
            return VK_IMAGE_LAYOUT_UNDEFINED;
        case ImageLayout::TransferSRC:
            return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        case ImageLayout::TransferDST:
            return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        case ImageLayout::RenderTarget:
            return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        case ImageLayout::Present:
            return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        default:
            return VK_IMAGE_LAYOUT_UNDEFINED;
    }
}

inline ImageLayout FromVkImageLayout(VkImageLayout layout) {
    switch (layout) {
        case VK_IMAGE_LAYOUT_UNDEFINED:
            return ImageLayout::Undefined;
        case VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL:
            return ImageLayout::TransferSRC;
        case VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL:
            return ImageLayout::TransferDST;
        case VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL:
            return ImageLayout::RenderTarget;
        case VK_IMAGE_LAYOUT_PRESENT_SRC_KHR:
            return ImageLayout::Present;
        default:
            return ImageLayout::Undefined;
    }
}

inline VkAccessFlags ToVkAccess(uint32_t flags) {
    VkAccessFlags vkFlags = VK_ACCESS_NONE;

    if (flags & ACCESS_SHADER_READ) {
        vkFlags |= VK_ACCESS_SHADER_READ_BIT;
    }
    if (flags & ACCESS_SHADER_WRITE) {
        vkFlags |= VK_ACCESS_SHADER_WRITE_BIT;
    }
    if (flags & ACCESS_COLOR_ATTACHMENT_READ) {
        vkFlags |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT;
    }
    if (flags & ACCESS_COLOR_ATTACHMENT_WRITE) {
        vkFlags |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }
    if (flags & ACCESS_DEPTH_STENCIL_ATTACHMENT_READ) {
        vkFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    }
    if (flags & ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE) {
        vkFlags |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
    if (flags & ACCESS_TRANSFER_READ) {
        vkFlags |= VK_ACCESS_TRANSFER_READ_BIT;
    }
    if (flags & ACCESS_TRANSFER_WRITE) {
        vkFlags |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }

    return vkFlags;
}

inline VkPipelineStageFlags ToVkStage(uint32_t flags) {
    VkPipelineStageFlags vkFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;


    if (flags & PIPELINE_STAGE_NONE) {
        vkFlags |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }
    if (flags & PIPELINE_STAGE_VERTEX_INPUT) {
        vkFlags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }
    if (flags & PIPELINE_STAGE_VERTEX_SHADER) {
        vkFlags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
    }
    if (flags & PIPELINE_STAGE_FRAGMENT_SHADER) {
        vkFlags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    if (flags & PIPELINE_STAGE_EARLY_FRAGMENT_TESTS) {
        vkFlags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    if (flags & PIPELINE_STAGE_LATE_FRAGMENT_TESTS) {
        vkFlags |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    }
    if (flags & PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT) {
        vkFlags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    }
    if (flags & PIPELINE_STAGE_COMPUTE_SHADER) {
        vkFlags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    }
    if (flags & PIPELINE_STAGE_TRANSFER) {
        vkFlags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    if (flags & PIPELINE_STAGE_ALL_GRAPHICS) {
        vkFlags |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT;
    }
    if (flags & PIPELINE_STAGE_ALL_COMMANDS) {
        vkFlags |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }

    return vkFlags;
}

inline VkImageUsageFlags ToVkImageUsageFlags(uint8_t flags) {
    VkImageUsageFlags vkFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    if (flags & TEXTURE_BIND_SHADER_RESOURCE) {
        vkFlags |= VK_IMAGE_USAGE_SAMPLED_BIT;
    }
    if (flags & TEXTURE_BIND_RENDER_TARGET) {
        vkFlags |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
    }
    if (flags & TEXTURE_BIND_DEPTH_STENCIL) {
        vkFlags |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
    }
    if (flags & TEXTURE_BIND_TRANSFER_SRC) {
        vkFlags |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
    }
    if (flags & TEXTURE_BIND_TRANSFER_DST) {
        vkFlags |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;
    }

    return vkFlags;
}

inline VkBufferUsageFlags ToVkBufferUsageFlags(uint8_t flags) {
    VkBufferUsageFlags vkFlags = VK_BUFFER_USAGE_TRANSFER_DST_BIT;

    if (flags & BUFFER_BIND_VERTEX) {
        vkFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    }
    if (flags & BUFFER_BIND_INDEX) {
        vkFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    }
    if (flags & BUFFER_BIND_UNIFORM) {
        vkFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    }
    if (flags & BUFFER_BIND_TRANSFER_SRC) {
        vkFlags |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    }
    if (flags & BUFFER_BIND_TRANSFER_DST) {
        vkFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;
    }

    return vkFlags;
}

} // vk

#endif //DERANGED_RHI_VULKANRESOURCE_H
