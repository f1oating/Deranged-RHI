//
// Created by alan on 08/08/2026.
//

#include "Backend/Vulkan/VulkanCommandQueue.h"

#include <cstring>
#include <spdlog/spdlog.h>
#include "VulkanResource.h"
#include "Backend/Vulkan/VulkanDevice.h"

namespace vk {

VulkanCommandQueue::VulkanCommandQueue(uint32_t queueIndex, VulkanDevice* device) {
    m_QueueIndex = queueIndex;
    m_Device = device;
    vkGetDeviceQueue(m_Device->GetVkDevice(), m_QueueIndex, 0, &m_Queue);
    m_CommandBufferPool = std::make_unique<CommandBufferPool>(m_Device->GetVkDevice(), m_QueueIndex);
    m_DescriptorManager = std::make_unique<DescriptorManager>(m_Device->GetVkDevice());
    m_Fence = new VulkanFence(m_Device);

    AcquireCommandBuffer();

    spdlog::info("VulkanQueue Created.");
}

VulkanCommandQueue::~VulkanCommandQueue() {
    if (m_Fence) {
        delete m_Fence;
    }
    m_DescriptorManager.reset();
    m_CommandBufferPool.reset();
    m_ReleaseManager.Clear();

    spdlog::info("VulkanQueue Destroyed.");
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

    m_DescriptorManager->SetDescriptorState(vkGraphicsPipelineState->GetDescriptorState());
    m_GraphicsPipeline = vkGraphicsPipelineState;
    m_GraphicsPipelineBound = false;
}

void VulkanCommandQueue::SetViewport(Viewport viewport) {
    m_Viewport = viewport;
    m_ViewportBound = false;
}

void VulkanCommandQueue::SetScissor(Scissor scissor) {
    m_Scissor = scissor;
    m_ScissorBound = false;
}

void VulkanCommandQueue::SetRenderTargets(std::vector<RenderTargetView*> rtvs) {
    m_RTVs.resize(rtvs.size());
    for (int i = 0; i < m_RTVs.size(); i++) {
        m_RTVs[i] = static_cast<VulkanRenderTargetView*>(rtvs[i]);
    }
}

void VulkanCommandQueue::SetDepthStencil(DepthStencilView* dsv) {
    m_DSV = static_cast<VulkanDepthStencilView*>(dsv);
}

void VulkanCommandQueue::ClearRenderTargets(float r, float g, float b, float a) {
    m_RTVsClearValue = { r, g, b, a };
    m_ShouldClearRTVs = true;
}

void VulkanCommandQueue::ClearDepthStencil(float depth, uint8_t stencil) {
    m_DSVClearValue = { depth, stencil };
    m_ShouldClearDSV = true;
}

void VulkanCommandQueue::SetVertexBuffer(Buffer* buffer) {
    m_VertexBuffer = static_cast<VulkanBuffer*>(buffer);
    m_VertexBufferBound = false;
}

void VulkanCommandQueue::SetIndexBuffer(Buffer* buffer) {
    m_IndexBuffer = static_cast<VulkanBuffer*>(buffer);
    m_IndexBufferBound = false;
}

void VulkanCommandQueue::SetConstantBuffer(std::string name, Buffer* buffer) {
    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
    const auto [set, binding] = m_GraphicsPipeline->GetBindingPlace(name);

    VkDescriptorBufferInfo bufferInfo = {
        .buffer = vkBuffer->GetVkBuffer(),
        .offset = vkBuffer->GetOffset(),
        .range = vkBuffer->GetDesc().Size
    };

    m_DescriptorManager->WriteBufferInfo(set, binding, bufferInfo);
}

void VulkanCommandQueue::SetTexture(std::string name, ShaderResourceView* textureView) {
    VulkanShaderResourceView* vkTextureView = static_cast<VulkanShaderResourceView*>(textureView);
    const auto [set, binding] = m_GraphicsPipeline->GetBindingPlace(name);

    VkDescriptorImageInfo imageInfo = {
        .imageView = vkTextureView->GetVkImageView(),
        .imageLayout = ToVkImageLayout(vkTextureView->GetVkTexture()->GetLayout())
    };

    m_DescriptorManager->WriteImageInfo(set, binding, imageInfo);
}

void VulkanCommandQueue::SetSampler(std::string name, Sampler* sampler) {
    VulkanSampler* vkSampler = static_cast<VulkanSampler*>(sampler);
    const auto [set, binding] = m_GraphicsPipeline->GetBindingPlace(name);

    VkDescriptorImageInfo imageInfo = {
        .sampler = vkSampler->GetVkSampler()
    };

    m_DescriptorManager->WriteImageInfo(set, binding, imageInfo);
}

void VulkanCommandQueue::DrawInstanced(uint32_t vertexCount, uint32_t instanceCount,
    uint32_t startVertex, uint32_t startInstance) {
    if (!m_InsideRendering) {
        BeginRendering();
    }

    BoundDirtyResources();
    ClearDirtyAttachmentsIfInsideRendering();

    m_DescriptorManager->WriteAndBind(m_CommandBuffer, m_GraphicsPipeline->GetVkLayout(), m_CommandBufferNumber);

    vkCmdDraw(m_CommandBuffer, vertexCount, instanceCount, startVertex, startInstance);
}

void VulkanCommandQueue::DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount,
        uint32_t startIndex, uint32_t vertexOffset, uint32_t startInstance) {
    if (!m_InsideRendering) {
        BeginRendering();
    }

    BoundDirtyResources();
    ClearDirtyAttachmentsIfInsideRendering();

    m_DescriptorManager->WriteAndBind(m_CommandBuffer, m_GraphicsPipeline->GetVkLayout(), m_CommandBufferNumber);

    vkCmdDrawIndexed(m_CommandBuffer, indexCount, instanceCount, startIndex, vertexOffset, startInstance);
}

void VulkanCommandQueue::Barrier(uint32_t srcStage, uint32_t dstStage,
    std::vector<BufferBarrier> bufBarriers, std::vector<TextureBarrier> texBarriers) {
    if (m_InsideRendering) {
        EndRendering();
    }

    std::vector<VkImageMemoryBarrier> imageBarriers;
    std::vector<VkBufferMemoryBarrier> bufferBarriers;

    for (auto barrier : texBarriers) {
        VulkanTexture* vkTexture = static_cast<VulkanTexture*>(barrier.Tex);
        VkImageSubresourceRange range = {
            .aspectMask = vkTexture->GetVkAspectFlags(),
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1
        };
        VkImageMemoryBarrier imageBarrier = {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
            .srcAccessMask = ToVkAccess(barrier.SrcAccessFlags),
            .dstAccessMask = ToVkAccess(barrier.DstAccessFlags),
            .oldLayout = ToVkImageLayout(vkTexture->GetLayout()),
            .newLayout = ToVkImageLayout(barrier.Layout),
            .image = vkTexture->GetVkImage(),
            .subresourceRange = range
        };
        imageBarriers.push_back(imageBarrier);
        vkTexture->SetLayout(barrier.Layout);
    }

    for (auto barrier : bufBarriers) {
        VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(barrier.Buf);

        VkBufferMemoryBarrier bufferBarrier = {
            .sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
            .srcAccessMask = ToVkAccess(barrier.SrcAccessFlags),
            .dstAccessMask = ToVkAccess(barrier.DstAccessFlags),
            .buffer = vkBuffer->GetVkBuffer(),
            .offset = 0,
            .size = vkBuffer->GetDesc().Size
        };
        bufferBarriers.push_back(bufferBarrier);
    }

    vkCmdPipelineBarrier(m_CommandBuffer, ToVkStage(srcStage), ToVkStage(dstStage),
        VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, bufferBarriers.size(),
        bufferBarriers.data(), imageBarriers.size(), imageBarriers.data());
}

void VulkanCommandQueue::CopyToBuffer(Buffer* dst, uint64_t size, void* data) {
    if (m_InsideRendering) {
        EndRendering();
    }

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

    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = m_Device->FindMemoryTypeIndex(memoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
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

void  VulkanCommandQueue::CopyToTexture(Texture* dst, uint64_t size, void* data) {
    if (m_InsideRendering) {
        EndRendering();
    }

    VulkanTexture* vkDst = static_cast<VulkanTexture*>(dst);

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

    VkMemoryAllocateInfo memoryAllocateInfo = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = memoryRequirements.size,
        .memoryTypeIndex = m_Device->FindMemoryTypeIndex(memoryRequirements.memoryTypeBits,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
    };

    vkAllocateMemory(m_Device->GetVkDevice(), &memoryAllocateInfo, nullptr, &memory);
    vkBindBufferMemory(m_Device->GetVkDevice(), staging, memory, 0);

    void* mapped = nullptr;
    vkMapMemory(m_Device->GetVkDevice(), memory, 0, size, 0, &mapped);
    memcpy(mapped, data, size);
    vkUnmapMemory(m_Device->GetVkDevice(), memory);

    VkImageSubresourceLayers subresource = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .mipLevel = 0,
        .baseArrayLayer = 0,
        .layerCount = 1
    };
    VkBufferImageCopy copyRegion = {
        .bufferOffset = 0,
        .imageSubresource = subresource,
        .imageExtent = { vkDst->GetDesc().Width, vkDst->GetDesc().Height, 1 }
    };
    vkCmdCopyBufferToImage(m_CommandBuffer, staging,
        vkDst->GetVkImage(), ToVkImageLayout(vkDst->GetLayout()), 1, &copyRegion);

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
    m_CommandBufferPool->Poll(completedFenceValue);
    m_DescriptorManager->Free(completedFenceValue);
    m_ReleaseManager.DiscardResources(completedFenceValue);
}

void VulkanCommandQueue::AcquireCommandBuffer() {
    m_CommandBuffer = m_CommandBufferPool->AcquireCommandBuffer();
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

    std::vector<VkPipelineStageFlags> pipelineStageFlagBits(m_WaitSemaphores.size(), VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .pNext = &timelineSubmitInfo,
        .waitSemaphoreCount = (uint32_t)m_WaitSemaphores.size(),
        .pWaitSemaphores = m_WaitSemaphores.data(),
        .pWaitDstStageMask = pipelineStageFlagBits.data(),
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

    m_CommandBufferPool->ReleaseCommandBuffer(m_CommandBuffer, m_CommandBufferNumber);
    m_ReleaseManager.DiscardStaleResources(m_CommandBufferNumber);
}

void VulkanCommandQueue::MarkResourcesDirty() {
    m_GraphicsPipelineBound = false;
    m_ViewportBound = false;
    m_ScissorBound = false;
    m_VertexBufferBound = false;
    m_IndexBufferBound = false;
}

void VulkanCommandQueue::BoundDirtyResources() {
    if (!m_GraphicsPipelineBound) {
        vkCmdBindPipeline(m_CommandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_GraphicsPipeline->GetVkPipeline());
    }
    if (!m_ViewportBound) {
        VkViewport vkViewport = {
            .x = m_Viewport.TopLeftX,
            .y = m_Viewport.TopLeftY,
            .width = m_Viewport.Width,
            .height = m_Viewport.Height,
            .minDepth = m_Viewport.MinDepth,
            .maxDepth = m_Viewport.MaxDepth
        };

        vkCmdSetViewport(m_CommandBuffer, 0, 1, &vkViewport);
    }
    if (!m_ScissorBound) {
        VkRect2D vkScissor = {
            .offset = { m_Scissor.Left, m_Scissor.Top },
            .extent = { (uint32_t)m_Scissor.Right, (uint32_t)m_Scissor.Bottom },
        };

        vkCmdSetScissor(m_CommandBuffer, 0, 1, &vkScissor);
    }
    if (!m_VertexBufferBound) {
        VkBuffer vkBuffers = { m_VertexBuffer->GetVkBuffer() };
        VkDeviceSize offset = 0;
        vkCmdBindVertexBuffers(m_CommandBuffer, 0, 1, &vkBuffers, &offset);
    }
    if (!m_IndexBufferBound) {
        VkDeviceSize offset = 0;
        vkCmdBindIndexBuffer(m_CommandBuffer, m_IndexBuffer->GetVkBuffer(), offset, VK_INDEX_TYPE_UINT32);
    }
}

void VulkanCommandQueue::BeginRendering() {
    if (m_InsideRendering) {
        EndRendering();
    }

    VkRect2D renderArea{};

    std::vector<VkRenderingAttachmentInfo> attachments;

    VkRenderingAttachmentInfo depthStencilAttachment;
    VkImageAspectFlags depthStencilAspectFlags = 0;

    for (auto rtv : m_RTVs) {
        VkClearValue clearValue;
        clearValue.color = m_RTVsClearValue;

        VkRenderingAttachmentInfo attachmentInfo = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = rtv->GetVkImageView(),
            .imageLayout = ToVkImageLayout(rtv->GetVkTexture()->GetLayout()),
            .loadOp = m_ShouldClearRTVs ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clearValue
        };
        attachments.push_back(attachmentInfo);
        renderArea.extent.width = rtv->GetVkTexture()->GetDesc().Width;
        renderArea.extent.height = rtv->GetVkTexture()->GetDesc().Height;
    }

    if (m_DSV) {
        VkClearValue clearValue {};
        clearValue.depthStencil = m_DSVClearValue;

        depthStencilAttachment = {
            .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
            .imageView = m_DSV->GetVkImageView(),
            .imageLayout = ToVkImageLayout(m_DSV->GetVkTexture()->GetLayout()),
            .loadOp = m_ShouldClearDSV ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .clearValue = clearValue
        };
        depthStencilAspectFlags = m_DSV->GetVkTexture()->GetVkAspectFlags();
    }

    VkRenderingInfo renderingInfo = {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = renderArea,
        .layerCount = 1,
        .colorAttachmentCount = (uint32_t)attachments.size(),
        .pColorAttachments = attachments.data(),
        .pDepthAttachment = depthStencilAspectFlags & VK_IMAGE_ASPECT_DEPTH_BIT ? &depthStencilAttachment : nullptr,
        .pStencilAttachment = depthStencilAspectFlags & VK_IMAGE_ASPECT_STENCIL_BIT ? &depthStencilAttachment : nullptr
    };
    vkCmdBeginRendering(m_CommandBuffer, &renderingInfo);

    m_ShouldClearRTVs = false;
    m_ShouldClearDSV = false;
    m_InsideRendering = true;
}

void VulkanCommandQueue::EndRendering() {
    if (!m_InsideRendering) {
        return;
    }
    vkCmdEndRendering(m_CommandBuffer);
    m_InsideRendering = false;
}

void VulkanCommandQueue::ClearDirtyAttachmentsIfInsideRendering() {
    if (!(m_ShouldClearRTVs || m_ShouldClearDSV)) {
        return;
    }

    std::vector<VkClearAttachment> attachments;
    VkRect2D rect{};

    if (m_ShouldClearRTVs) {
        for (int i = 0; i < m_RTVs.size(); i++) {
            VkClearValue clearValue{};
            clearValue.color = m_RTVsClearValue;
            VkClearAttachment attachment = {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .colorAttachment = (uint32_t)i,
                .clearValue = clearValue
            };
            attachments.push_back(attachment);
            TextureDesc texDesc = m_RTVs[i]->GetVkTexture()->GetDesc();
            rect.extent = { texDesc.Width, texDesc.Height };
        }
    }

    if (m_ShouldClearDSV) {
        VkClearValue clearValue{};
        clearValue.depthStencil = m_DSVClearValue;
        VkClearAttachment attachment = {
            .aspectMask = m_DSV->GetVkTexture()->GetVkAspectFlags(),
            .clearValue = clearValue
        };
        attachments.push_back(attachment);
    }

    VkClearRect clearRect = {
        .rect = rect,
        .baseArrayLayer = 0,
        .layerCount = 1
    };
    vkCmdClearAttachments(m_CommandBuffer, attachments.size(),
        attachments.data(), 1, &clearRect);
}

} // vk