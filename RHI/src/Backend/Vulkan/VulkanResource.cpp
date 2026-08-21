//
// Created by alan on 13/08/2026.
//

#include "Backend/Vulkan/VulkanResource.h"
#include "Backend/Vulkan/VulkanDevice.h"

namespace vk {

VulkanTexture::VulkanTexture(TextureDesc desc, VulkanDevice* device) {
    m_Desc = desc;
    m_Device = device;

    VkImageCreateInfo imageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = ToVkImageType(m_Desc.Type),
        .format = ToVkFormat(m_Desc.Format),
        .extent = { m_Desc.Width, m_Desc.Height, 1 },
        .mipLevels = m_Desc.MipLevels,
        .arrayLayers = m_Desc.ArrayLayers,
        .samples = ToVkSampleCountFlagBits(m_Desc.Samples),
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    vkCreateImage(m_Device->GetVkDevice(), &imageCreateInfo, nullptr, &m_Image);

    VkPhysicalDeviceMemoryProperties physicalDeviceMemoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_Device->GetVkPhysicalDevice(), &physicalDeviceMemoryProperties);

    uint32_t memoryTypeIndex = 0;
    for (int i = 0; i < physicalDeviceMemoryProperties.memoryTypeCount; i++) {
        if (physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT) {
            memoryTypeIndex = i;
            break;
        }
    }

    VkMemoryRequirements imageMemoryRequirements;
    vkGetImageMemoryRequirements(m_Device->GetVkDevice(), m_Image, &imageMemoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = imageMemoryRequirements.size,
        .memoryTypeIndex = memoryTypeIndex
    };

    vkAllocateMemory(m_Device->GetVkDevice(), &memoryAllocateInfo, nullptr, &m_Memory);
    vkBindImageMemory(m_Device->GetVkDevice(), m_Image, m_Memory, 0);
}

VulkanTexture::VulkanTexture(TextureDesc desc, VulkanDevice* device, VkImage image) {
    m_Desc = desc;
    m_Device = device;
    m_Image = image;
}

VulkanTexture::~VulkanTexture() {
    if (m_RTV) {
        delete m_RTV;
    }
    if (m_Memory) {
        m_Device->ReleaseResource(new ImageReleaseResource(m_Device->GetVkDevice(), m_Image, m_Memory));
    }
}

TextureView* VulkanTexture::GetRTV() {
    if (!m_RTV) {
        TextureViewDesc desc = {
            .Tex = this,
            .Format = TextureFormat::B8G8R8A8_UNORM
        };
        m_RTV = new VulkanTextureView(desc, m_Device);
    }
    return m_RTV;
}

TextureDesc VulkanTexture::GetDesc() {
    return m_Desc;
}

VulkanTextureView::VulkanTextureView(TextureViewDesc desc, VulkanDevice* device) {
    m_Desc = desc;
    m_Device = device;

    VulkanTexture* texture = static_cast<VulkanTexture*>(m_Desc.Tex);
    m_Texture = texture;

    VkImageSubresourceRange imageSubresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };

    VkImageViewCreateInfo imageViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_Texture->GetVkImage(),
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = VK_FORMAT_B8G8R8A8_UNORM,
        .components = VK_COMPONENT_SWIZZLE_IDENTITY,
        .subresourceRange = imageSubresourceRange
    };

    vkCreateImageView(m_Device->GetVkDevice(), &imageViewCreateInfo, nullptr, &m_ImageView);
}

VulkanTextureView::~VulkanTextureView() {
    if (m_ImageView) {
        m_Device->ReleaseResource(new ImageViewReleaseResource(m_Device->GetVkDevice(), m_ImageView));
    }
}

TextureViewDesc VulkanTextureView::GetDesc() {
    return m_Desc;
}

} // vk