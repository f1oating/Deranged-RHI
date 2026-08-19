//
// Created by alan on 08/08/2026.
//

#include "Backend/Vulkan/VulkanCommandQueue.h"
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

}

void VulkanCommandQueue::SetRenderTargets(std::vector<TextureView*> rtvs) {

}

void VulkanCommandQueue::DrawInstaned(uint32_t VertexCountPerInstance, uint32_t InstanceCount,
    uint32_t StartVertexLocation, uint32_t StartInstanceLocation) {
    vkCmdDraw(m_CommandBuffer, VertexCountPerInstance, InstanceCount, StartVertexLocation, StartInstanceLocation);
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

    m_CommandBufferPool.ReleaseCommandBuffer(m_CommandBuffer, m_CommandBufferNumber);
    m_ReleaseManager.DiscardStaleResources(m_CommandBufferNumber);
}

} // vk