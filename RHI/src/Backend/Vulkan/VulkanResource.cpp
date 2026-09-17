//
// Created by alan on 13/08/2026.
//

#include "Backend/Vulkan/VulkanResource.h"
#include "Backend/Vulkan/VulkanDevice.h"
#include <spdlog/spdlog.h>

namespace vk {

VulkanTexture::VulkanTexture(TextureDesc desc, VulkanDevice* device) {
    m_Desc = desc;
    m_Device = device;
    m_AspectFlags = ToVkImageAspectFlags(m_Desc.Format);

    CreateTexture();
    CreateMemory();

    vkBindImageMemory(m_Device->GetVkDevice(), m_Image, m_Memory, 0);

    spdlog::info("VulkanTexture Created.");
}

VulkanTexture::VulkanTexture(TextureDesc desc, VulkanDevice* device, VkImage image) {
    m_Desc = desc;
    m_Device = device;
    m_Image = image;

    spdlog::info("VulkanTexture Created.");
}

VulkanTexture::~VulkanTexture() {
    if (m_RTV) {
        delete m_RTV;
    }
    if (m_DSV) {
        delete m_DSV;
    }
    if (m_SRV) {
        delete m_SRV;
    }
    if (m_Memory) {
        m_Device->ReleaseResource(new ImageReleaseResource(m_Device->GetVkDevice(), m_Image, m_Memory));
    }

    spdlog::info("VulkanTexture Destroyed.");
}

RenderTargetView* VulkanTexture::GetRTV() {
    if (!m_RTV) {
        m_RTV = new VulkanRenderTargetView(this, m_Device);
    }
    return m_RTV;
}

DepthStencilView* VulkanTexture::GetDSV() {
    if (!m_DSV) {
        m_DSV = new VulkanDepthStencilView(this, m_Device);
    }
    return m_DSV;
}

ShaderResourceView* VulkanTexture::GetSRV() {
    if (!m_SRV) {
        m_SRV = new VulkanShaderResourceView(this, m_Device);
    }
    return m_SRV;
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
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = ToVkImageUsageFlags(m_Desc.BindFlags),
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    vkCreateImage(m_Device->GetVkDevice(), &imageCreateInfo, nullptr, &m_Image);
}

void VulkanTexture::CreateMemory() {
    VkMemoryRequirements imageMemoryRequirements;
    vkGetImageMemoryRequirements(m_Device->GetVkDevice(), m_Image, &imageMemoryRequirements);

    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = imageMemoryRequirements.size,
        .memoryTypeIndex = m_Device->FindMemoryTypeIndex(imageMemoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    };

    vkAllocateMemory(m_Device->GetVkDevice(), &memoryAllocateInfo, nullptr, &m_Memory);
}

VulkanRenderTargetView::VulkanRenderTargetView(VulkanTexture* texture, VulkanDevice* device) {
    m_Device = device;
    m_Texture = texture;

    VkImageSubresourceRange imageSubresourceRange = {
        .aspectMask = texture->GetVkAspectFlags(),
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };

    VkImageViewCreateInfo imageViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_Texture->GetVkImage(),
        .viewType = ToVkImageViewType(texture->GetDesc().Type),
        .format = ToVkFormat(m_Texture->GetDesc().Format),
        .components = VK_COMPONENT_SWIZZLE_IDENTITY,
        .subresourceRange = imageSubresourceRange
    };

    vkCreateImageView(m_Device->GetVkDevice(), &imageViewCreateInfo, nullptr, &m_View);

    spdlog::info("VulkanRenderTargetView Created.");
}

VulkanRenderTargetView::~VulkanRenderTargetView() {
    if (m_View) {
        m_Device->ReleaseResource(new ImageViewReleaseResource(m_Device->GetVkDevice(), m_View));
    }

    spdlog::info("VulkanRenderTargetView Destroyed.");
}

VulkanDepthStencilView::VulkanDepthStencilView(VulkanTexture* texture, VulkanDevice* device) {
    m_Device = device;
    m_Texture = texture;

    VkImageSubresourceRange imageSubresourceRange = {
        .aspectMask = texture->GetVkAspectFlags(),
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };

    VkImageViewCreateInfo imageViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_Texture->GetVkImage(),
        .viewType = ToVkImageViewType(texture->GetDesc().Type),
        .format = ToVkFormat(m_Texture->GetDesc().Format),
        .components = VK_COMPONENT_SWIZZLE_IDENTITY,
        .subresourceRange = imageSubresourceRange
    };

    vkCreateImageView(m_Device->GetVkDevice(), &imageViewCreateInfo, nullptr, &m_View);

    spdlog::info("VulkanDepthStencilView Created.");
}

VulkanDepthStencilView::~VulkanDepthStencilView() {
    if (m_View) {
        m_Device->ReleaseResource(new ImageViewReleaseResource(m_Device->GetVkDevice(), m_View));
    }

    spdlog::info("VulkanDepthStencilView Destroyed.");
}

VulkanShaderResourceView::VulkanShaderResourceView(VulkanTexture* texture, VulkanDevice* device) {
    m_Device = device;
    m_Texture = texture;

    VkImageSubresourceRange imageSubresourceRange = {
        .aspectMask = texture->GetVkAspectFlags(),
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1
    };

    VkImageViewCreateInfo imageViewCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = m_Texture->GetVkImage(),
        .viewType = ToVkImageViewType(texture->GetDesc().Type),
        .format = ToVkFormat(m_Texture->GetDesc().Format),
        .components = VK_COMPONENT_SWIZZLE_IDENTITY,
        .subresourceRange = imageSubresourceRange
    };

    vkCreateImageView(m_Device->GetVkDevice(), &imageViewCreateInfo, nullptr, &m_View);

    spdlog::info("VulkanShaderResourceView Created.");
}

VulkanShaderResourceView::~VulkanShaderResourceView() {
    if (m_View) {
        m_Device->ReleaseResource(new ImageViewReleaseResource(m_Device->GetVkDevice(), m_View));
    }

    spdlog::info("VulkanShaderResourceView Destroyed.");
}

VulkanBuffer::VulkanBuffer(BufferDesc desc, VulkanDevice* device) {
    m_Desc = desc;
    m_Device = device;

    CreateBuffer();
    CreateMemory();

    vkBindBufferMemory(m_Device->GetVkDevice(), m_Buffer, m_Memory, 0);

    spdlog::info("VulkanBuffer Created.");
}

VulkanBuffer::~VulkanBuffer() {
    if (m_Desc.Usage == BufferUsage::Dynamic) {
        m_Device->ReleaseResource(new BufferReleaseResource(m_Device->GetVkDevice(), m_Buffer, nullptr));
    } else {
        m_Device->ReleaseResource(new BufferReleaseResource(m_Device->GetVkDevice(), m_Buffer, m_Memory));
    }

    spdlog::info("VulkanBuffer Destroyed.");
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
        .size = m_Desc.Usage != BufferUsage::Dynamic ? m_Desc.Size : 512 * 512 * 4,
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

VulkanSampler::VulkanSampler(SamplerDesc desc, VulkanDevice* device) {
    m_Device = device;
    m_Desc = desc;

    VkSamplerCreateInfo createInfo = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = ToVkFilter(m_Desc.Filtering),
        .minFilter = ToVkFilter(m_Desc.Filtering),
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_NEAREST,
        .addressModeU = ToVkSamplerAddressMode(m_Desc.AddressU),
        .addressModeV = ToVkSamplerAddressMode(m_Desc.AddressV),
        .addressModeW = ToVkSamplerAddressMode(m_Desc.AddressW),
        .mipLodBias = m_Desc.MipLodBias,
        .anisotropyEnable = m_Desc.MaxAnisotropy,
        .maxAnisotropy = (float)m_Desc.MaxAnisotropy,
        .compareEnable = m_Desc.Compare != CompareOp::Never,
        .compareOp = ToVkCompareOp(m_Desc.Compare),
        .minLod = m_Desc.MinLod,
        .maxLod = m_Desc.MaxLod,
        .borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK,
        .unnormalizedCoordinates = false
    };

    vkCreateSampler(m_Device->GetVkDevice(), &createInfo, nullptr, &m_Sampler);

    spdlog::info("VulkanSampler Created.");
}

VulkanSampler::~VulkanSampler() {
    if (m_Sampler) {
        m_Device->ReleaseResource(new SamplerReleaseResource(m_Device->GetVkDevice(), m_Sampler));
    }

    spdlog::info("VulkanSampler Destroyed.");
}

SamplerDesc VulkanSampler::GetDesc() {
    return m_Desc;
}

} // vk