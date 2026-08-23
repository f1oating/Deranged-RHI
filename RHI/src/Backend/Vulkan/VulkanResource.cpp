//
// Created by alan on 13/08/2026.
//

#include "Backend/Vulkan/VulkanResource.h"
#include "Backend/Vulkan/VulkanDevice.h"

namespace vk {

VulkanTexture::VulkanTexture(TextureDesc desc, VulkanDevice* device) {
    m_Desc = desc;
    m_Device = device;

    CreateTexture();
    CreateMemory();

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

void VulkanTexture::CreateTexture() {
    VkImageCreateInfo imageCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = ToVkImageType(m_Desc.Type),
        .format = ToVkFormat(m_Desc.Format),
        .extent = { m_Desc.Width, m_Desc.Height, 1 },
        .mipLevels = m_Desc.MipLevels,
        .arrayLayers = m_Desc.ArrayLayers,
        .samples = ToVkSampleCountFlagBits(m_Desc.Samples),
        .tiling = VK_IMAGE_TILING_LINEAR,
        .usage = ToVkImageUsageFlags(m_Desc.BindFlags),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    vkCreateImage(m_Device->GetVkDevice(), &imageCreateInfo, nullptr, &m_Image);
}

void VulkanTexture::CreateMemory() {
    VkMemoryRequirements imageMemoryRequirements;
    vkGetImageMemoryRequirements(m_Device->GetVkDevice(), m_Image, &imageMemoryRequirements);
    m_SizeInBytes = imageMemoryRequirements.size;

    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = m_SizeInBytes,
        .memoryTypeIndex = m_Device->FindMemoryTypeIndex(imageMemoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    };

    vkAllocateMemory(m_Device->GetVkDevice(), &memoryAllocateInfo, nullptr, &m_Memory);
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
        .format = ToVkFormat(m_Desc.Format),
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

VulkanBuffer::VulkanBuffer(BufferDesc desc, VulkanDevice* device) {
    m_Desc = desc;
    m_Device = device;

    CreateBuffer();
    CreateMemory();

    vkBindBufferMemory(m_Device->GetVkDevice(), m_Buffer, m_Memory, 0);
}

VulkanBuffer::~VulkanBuffer() {
    if (m_Desc.Usage == BufferUsage::Dynamic) {
        m_Device->ReleaseResource(new BufferReleaseResource(m_Device->GetVkDevice(), m_Buffer, nullptr));
    } else {
        m_Device->ReleaseResource(new BufferReleaseResource(m_Device->GetVkDevice(), m_Buffer, m_Memory));
    }
}

void* VulkanBuffer::Map() {
    m_Offset = m_Device->GetRingBuffer()->Allocate(m_SizeInBytes);
    return (uint8_t*)m_Mapped + m_Offset;
}

BufferDesc VulkanBuffer::GetDesc() {
    return m_Desc;
}

void VulkanBuffer::CreateBuffer() {
    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = m_Desc.Size,
        .usage = ToVkBufferUsageFlags(m_Desc.BindFlags),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };

    vkCreateBuffer(m_Device->GetVkDevice(), &bufferCreateInfo, nullptr, &m_Buffer);
}

void VulkanBuffer::CreateMemory() {
    VkMemoryRequirements bufferMemoryRequirements;
    vkGetBufferMemoryRequirements(m_Device->GetVkDevice(), m_Buffer, &bufferMemoryRequirements);
    m_SizeInBytes = bufferMemoryRequirements.size;

    if (m_Desc.Usage == BufferUsage::Dynamic) {
        m_Memory = m_Device->GetRingBuffer()->GetMemory();
        m_Mapped = m_Device->GetRingBuffer()->GetMapped();
        return;
    }

    uint32_t memoryPropertyFlags = m_Desc.Usage == BufferUsage::Default ?
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT :
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = m_SizeInBytes,
        .memoryTypeIndex = m_Device->FindMemoryTypeIndex(bufferMemoryRequirements.memoryTypeBits, memoryPropertyFlags)
    };

    vkAllocateMemory(m_Device->GetVkDevice(), &memoryAllocateInfo, nullptr, &m_Memory);
}

} // vk