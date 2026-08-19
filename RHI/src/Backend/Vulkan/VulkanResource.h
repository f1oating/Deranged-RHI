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

    VkImage GetVkImage() const { return m_Image; }

private:
    VulkanDevice* m_Device = nullptr;
    TextureDesc m_Desc;
    VkImage m_Image = nullptr;
    VkDeviceMemory m_Memory = nullptr;

};

class VulkanTextureView : public TextureView {
public:
    VulkanTextureView(TextureViewDesc desc, VulkanDevice* device);
    ~VulkanTextureView() override;

private:
    VulkanDevice* m_Device = nullptr;
    TextureViewDesc m_Desc;
    VkImageView m_ImageView = nullptr;

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

inline VkFormat ToVkFormat(TextureFormat format) {
    switch (format) {
        case TextureFormat::TEXTURE_FORMAT_B8G8R8A8_UNORM:
            return VK_FORMAT_B8G8R8A8_UNORM;
        default:
            return VK_FORMAT_B8G8R8A8_UNORM;
    }
}

inline TextureFormat FromVkFormat(VkFormat format) {
    switch (format) {
        case VK_FORMAT_B8G8R8A8_UNORM:
            return TextureFormat::TEXTURE_FORMAT_B8G8R8A8_UNORM;
        default:
            return TextureFormat::TEXTURE_FORMAT_B8G8R8A8_UNORM;
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

} // vk

#endif //DERANGED_RHI_VULKANRESOURCE_H
