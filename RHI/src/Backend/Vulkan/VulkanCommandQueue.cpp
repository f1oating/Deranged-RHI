//
// Created by alan on 08/08/2026.
//

#include "Backend/Vulkan/VulkanCommandQueue.h"

#include <cstring>

#include "VulkanResource.h"
#include "Backend/Vulkan/VulkanDevice.h"

namespace vk {

VulkanCommandQueue::VulkanCommandQueue(uint32_t queueIndex, VulkanDevice* device) {
    m_QueueIndex = queueIndex;
    m_Device = device;
    vkGetDeviceQueue(m_Device->GetVkDevice(), m_QueueIndex, 0, &m_Queue);
    m_CommandBufferPool.Init(m_Device->GetVkDevice(), m_QueueIndex);
    m_Fence = new VulkanFence(m_Device);

    AcquireCommandBuffer();
}

VulkanCommandQueue::~VulkanCommandQueue() {
    if (m_Fence) {
        delete m_Fence;
    }
    m_CommandBufferPool.Shutdown();
    m_ReleaseManager.Clear();
}

void VulkanCommandQueue::Wait(Fence* fence, uint64_t value) {
    VulkanFence* vulkanFence = static_cast<VulkanFence*>(fence);
    AddWaitSemaphore(vulkanFence->GetVkSemaphore(), value);
}

void VulkanCommandQueue::Signal(Fence* fence, uint64_t value) {
    VulkanFence* vulkanFence = static_cast<VulkanFence*>(fence);
    AddSignalSemaphore(vulkanFence->GetVkSemaphore(), value);
}

void VulkanCommandQueue::SetGraphicsPipelineState(GraphicsPipelineState* graphicsPipelineState) {
    VulkanGraphicsPipelineState* vkGraphicsPipelineState = static_cast<VulkanGraphicsPipelineState*>(graphicsPipelineState);

    vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vkGraphicsPipelineState->GetPipeline());
}

void VulkanCommandQueue::SetViewport(Viewport viewport) {
    VkViewport vkViewport = {
        .x = viewport.TopLeftX,
        .y = viewport.TopLeftY,
        .width = viewport.Width,
        .height = viewport.Height,
        .minDepth = viewport.MinDepth,
        .maxDepth = viewport.MaxDepth
    };

    vkCmdSetViewport(m_CommandBuffer, 0, 1, &vkViewport);
}

void VulkanCommandQueue::SetScissor(Scissor scissor) {
    VkRect2D vkScissor = {
        .offset = { scissor.Left, scissor.Top },
        .extent = { (uint32_t)scissor.Right, (uint32_t)scissor.Bottom },
    };

    vkCmdSetScissor(m_CommandBuffer, 0, 1, &vkScissor);
}

void VulkanCommandQueue::Barrier(std::vector<TextureBarrier> barriers) {
    if (m_InsideRendering) {
        EndRendering();
    }

    std::vector<VkImageMemoryBarrier> imageBarriers;
    for (auto barrier : barriers) {
        VulkanTexture* vkTexture = static_cast<VulkanTexture*>(barrier.Tex);
        VkImageSubresourceRange range = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };
        VkImageMemoryBarrier imageBarrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = ToSrcVkAccessFlags(barrier.Layout),
            .dstAccessMask = ToDstVkAccessFlags(barrier.Layout),
            .oldLayout = ToVkImageLayout(vkTexture->GetLayout()),
            .newLayout = ToVkImageLayout(barrier.Layout),
            .image = vkTexture->GetVkImage(),
            .subresourceRange = range
        };
        imageBarriers.push_back(imageBarrier);
        vkTexture->SetLayout(barrier.Layout);
    }

    vkCmdPipelineBarrier(m_CommandBuffer, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
        VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, imageBarriers.size(), imageBarriers.data());
}

void VulkanCommandQueue::SetRenderTargets(std::vector<TextureView*> rtvs) {
    m_RTVs.resize(rtvs.size());
    for (int i = 0; i < m_RTVs.size(); i++) {
        m_RTVs[i] = static_cast<VulkanTextureView*>(rtvs[i]);
    }
    BeginRendering();
}

void VulkanCommandQueue::ClearRenderTargets(float r, float g, float b, float a) {
    if (!m_InsideRendering) {
        BeginRendering();
    }

    std::vector<VkClearAttachment> attachments;
    VkRect2D rect{};

    for (int i = 0; i < m_RTVs.size(); i++) {
        VkClearAttachment attachment = {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .colorAttachment = (uint32_t)i,
            .clearValue = { r, g, b, a }
        };
        attachments.push_back(attachment);
        TextureDesc texDesc = m_RTVs[i]->GetTexture()->GetDesc();
        rect.extent = { texDesc.Width, texDesc.Height };
    }

    VkClearRect clearRect = {
        .rect = rect,
        .baseArrayLayer = 0,
        .layerCount = 1
    };
    vkCmdClearAttachments(m_CommandBuffer, attachments.size(),
        attachments.data(), 1, &clearRect);
}

void VulkanCommandQueue::DrawInstansed(uint32_t VertexCountPerInstance, uint32_t InstanceCount,
    uint32_t StartVertexLocation, uint32_t StartInstanceLocation) {
    if (!m_InsideRendering) {
        BeginRendering();
    }
    vkCmdDraw(m_CommandBuffer, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
}

void VulkanCommandQueue::CopyToBuffer(Buffer* dst, uint64_t size, void* data) {
    VulkanBuffer* vkDst = static_cast<VulkanBuffer*>(dst);

    VkBuffer staging = nullptr;
    VkDeviceMemory memory = nullptr;

    VkBufferCreateInfo bufferCreateInfo = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };
    vkCreateBuffer(m_Device->GetVkDevice(), &bufferCreateInfo, nullptr, &staging);

    VkMemoryRequirements memoryRequirements;
    vkGetBufferMemoryRequirements(m_Device->GetVkDevice(), staging, &memoryRequirements);

    VkPhysicalDeviceMemoryProperties memoryProperties;
    vkGetPhysicalDeviceMemoryProperties(m_Device->GetVkPhysicalDevice(), &memoryProperties);

    uint32_t memoryTypeIndex = 0;
    for (int i = 0; i < memoryProperties.memoryTypeCount; i++) {
        if (memoryProperties.memoryTypes[i].propertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) {
            memoryTypeIndex = i;
            break;
        }
    }

    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = memoryTypeIndex,
    };

    vkAllocateMemory(m_Device->GetVkDevice(), &memoryAllocateInfo, nullptr, &memory);
    vkBindBufferMemory(m_Device->GetVkDevice(), staging, memory, 0);

    void* mapped = nullptr;
    vkMapMemory(m_Device->GetVkDevice(), memory, 0, size, 0, &mapped);
    memcpy(mapped, data, size);
    vkUnmapMemory(m_Device->GetVkDevice(), memory);

    VkBufferCopy copyRegion = {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = size
    };
    vkCmdCopyBuffer(m_CommandBuffer, staging, vkDst->GetVkBuffer(), 1, &copyRegion);

    ReleaseResource(new ReleaseResourceWrapper(new BufferReleaseResource(m_Device->GetVkDevice(), staging, memory)));
}

void VulkanCommandQueue::Flush() {
    SubmitCommandBuffer();
    AcquireCommandBuffer();
}

void VulkanCommandQueue::AddWaitSemaphore(VkSemaphore waitSemaphore, uint64_t value) {
    m_WaitSemaphores.push_back(waitSemaphore);
    m_WaitSemaphoresValues.push_back(value);
}

void VulkanCommandQueue::AddSignalSemaphore(VkSemaphore signalSemaphore, uint64_t value) {
    m_SignalSemaphores.push_back(signalSemaphore);
    m_SignalSemaphoresValues.push_back(value);
}

void VulkanCommandQueue::ReleaseResource(ReleaseResourceWrapper* releaseResourceWrapper) {
    m_ReleaseManager.ReleaseResource(releaseResourceWrapper);
}

void VulkanCommandQueue::EndFrame() {
    uint64_t completedFenceValue = m_Fence->GetCompletedValue();
    m_CommandBufferPool.Poll(completedFenceValue);
    m_ReleaseManager.DiscardResources(completedFenceValue);
}

void VulkanCommandQueue::AcquireCommandBuffer() {
    m_CommandBuffer = m_CommandBufferPool.AcquireCommandBuffer();
    vkResetCommandBuffer(m_CommandBuffer, 0);

    VkCommandBufferBeginInfo commandBufferBeginInfo = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    };
    vkBeginCommandBuffer(m_CommandBuffer, &commandBufferBeginInfo);
    m_CommandBufferNumber++;
}

void VulkanCommandQueue::SubmitCommandBuffer() {
    EndRendering();
    vkEndCommandBuffer(m_CommandBuffer);

    AddSignalSemaphore(m_Fence->GetVkSemaphore(), m_CommandBufferNumber);

    VkTimelineSemaphoreSubmitInfo timelineSubmitInfo = {
        .sType = VK_STRUCTURE_TYPE_TIMELINE_SEMAPHORE_SUBMIT_INFO,
        .waitSemaphoreValueCount = (uint32_t)m_WaitSemaphoresValues.size(),
        .pWaitSemaphoreValues = m_WaitSemaphoresValues.data(),
        .signalSemaphoreValueCount = (uint32_t)m_SignalSemaphores.size(),
        .pSignalSemaphoreValues = m_SignalSemaphoresValues.data()
    };

    VkPipelineStageFlags pipelineStageFlagBits = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = &timelineSubmitInfo,
        .waitSemaphoreCount = (uint32_t)m_WaitSemaphores.size(),
        .pWaitSemaphores = m_WaitSemaphores.data(),
        .pWaitDstStageMask = &pipelineStageFlagBits,
        .commandBufferCount = 1,
        .pCommandBuffers = &m_CommandBuffer,
        .signalSemaphoreCount = (uint32_t)m_SignalSemaphores.size(),
        .pSignalSemaphores = m_SignalSemaphores.data()
    };
    vkQueueSubmit(m_Queue, 1, &submitInfo, nullptr);

    m_WaitSemaphores.clear();
    m_WaitSemaphoresValues.clear();
    m_SignalSemaphores.clear();
    m_SignalSemaphoresValues.clear();
    m_RTVs.clear();

    m_CommandBufferPool.ReleaseCommandBuffer(m_CommandBuffer, m_CommandBufferNumber);
    m_ReleaseManager.DiscardStaleResources(m_CommandBufferNumber);
}

void VulkanCommandQueue::BeginRendering() {
    if (m_InsideRendering) {
        EndRendering();
    }

    VkRect2D renderArea{};
    std::vector<VkRenderingAttachmentInfo> attachments;

    for (auto rtv : m_RTVs) {
        VkClearValue clearValue = { 0.0f, 0.0f, 0.0f, 1.0f };
        VkRenderingAttachmentInfo attachmentInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = rtv->GetVkImageView(),
            .imageLayout = ToVkImageLayout(rtv->GetTexture()->GetLayout()),
            .loadOp = VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clearValue
        };
        attachments.push_back(attachmentInfo);
        renderArea.extent.width = rtv->GetTexture()->GetDesc().Width;
        renderArea.extent.height = rtv->GetTexture()->GetDesc().Height;
    }

    VkRenderingInfo renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = renderArea,
        .layerCount = 1,
        .colorAttachmentCount = (uint32_t)attachments.size(),
        .pColorAttachments = attachments.data(),
        .pDepthAttachment = nullptr,
        .pStencilAttachment = nullptr,
    };
    vkCmdBeginRendering(m_CommandBuffer, &renderingInfo);
    m_InsideRendering = true;
}

void VulkanCommandQueue::EndRendering() {
    if (!m_InsideRendering) {
        return;
    }
    vkCmdEndRendering(m_CommandBuffer);
    m_InsideRendering = false;
}

} // vk