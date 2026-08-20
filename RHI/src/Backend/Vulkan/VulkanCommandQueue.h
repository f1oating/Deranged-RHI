//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_VULKANCOMMANDQUEUE_H
#define DERANGED_RHI_VULKANCOMMANDQUEUE_H

#include <volk.h>
#include "CommandQueue.h"
#include "Backend/Vulkan/Internal/CommandBufferPool.h"
#include <vector>
#include "Backend/Vulkan/VulkanFence.h"
#include "ReleaseManager.h"
#include "Backend/Vulkan/VulkanPipeline.h"
#include "Backend/Vulkan/VulkanResource.h"

namespace vk {

class VulkanDevice;

class VulkanCommandQueue : public CommandQueue {
public:
    VulkanCommandQueue(uint32_t queueIndex, VulkanDevice* device);
    ~VulkanCommandQueue() override;

    void Wait(Fence* fence, uint64_t value) override;
    void Signal(Fence* fence, uint64_t value) override;

    void SetGraphicsPipelineState(GraphicsPipelineState* graphicsPipelineState) override;

    void SetViewport(Viewport viewport) override;
    void SetScissor(Scissor scissor) override;

    void Barrier(std::vector<TextureBarrier> barriers) override;

    void SetRenderTargets(std::vector<TextureView*> rtvs) override;
    void ClearRenderTargets(float r, float g, float b, float a) override;

    void DrawInstansed(uint32_t VertexCountPerInstance, uint32_t InstanceCount = 1,
        uint32_t StartVertexLocation = 0, uint32_t StartInstanceLocation = 0) override;

    void Flush() override;

    void AddWaitSemaphore(VkSemaphore waitSemaphore, uint64_t value = 1);
    void AddSignalSemaphore(VkSemaphore signalSemaphore, uint64_t value = 1);

    void ReleaseResource(ReleaseResourceWrapper* releaseResourceWrapper);
    void EndFrame();

    uint32_t GetQueueFamilyIndex() const { return m_QueueIndex; };
    VkQueue GetVkQueue() const { return m_Queue; };

private:
    void AcquireCommandBuffer();
    void SubmitCommandBuffer();

    void BeginRendering();
    void EndRendering();

private:
    uint32_t m_QueueIndex = 0;
    VulkanDevice* m_Device;
    VkQueue m_Queue = nullptr;
    vk::CommandBufferPool m_CommandBufferPool;
    VkCommandBuffer m_CommandBuffer = nullptr;
    uint64_t m_CommandBufferNumber = 0;
    std::vector<VkSemaphore> m_WaitSemaphores;
    std::vector<uint64_t> m_WaitSemaphoresValues;
    std::vector<VkSemaphore> m_SignalSemaphores;
    std::vector<uint64_t> m_SignalSemaphoresValues;
    VulkanFence* m_Fence = nullptr;
    ReleaseManager m_ReleaseManager;
    std::vector<VulkanTextureView*> m_RTVs;
    bool m_InsideRendering = false;

};

} // vk

#endif //DERANGED_RHI_VULKANCOMMANDQUEUE_H
