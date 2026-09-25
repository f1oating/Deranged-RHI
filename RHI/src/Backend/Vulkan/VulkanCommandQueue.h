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
#include "Backend/Vulkan/Internal/DescriptorPool.h"

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

    void SetRenderTargets(std::vector<RenderTargetView*> rtvs) override;
    void SetDepthStencil(DepthStencilView* dsv) override;

    void ClearRenderTargets(float r, float g, float b, float a) override;
    void ClearDepthStencil(float depth, uint8_t stencil) override;

    void SetVertexBuffer(Buffer* buffer) override;
    void SetIndexBuffer(Buffer* buffer) override;

    void SetConstantBuffer(std::string name, Buffer* buffer) override;
    void SetTexture(std::string name, ShaderResourceView* textureView) override;
    void SetSampler(std::string name, Sampler* sampler) override;

    void DrawInstanced(uint32_t vertexCount, uint32_t instanceCount,
        uint32_t startVertex, uint32_t startInstance) override;
    void DrawIndexedInstanced(uint32_t indexCount, uint32_t instanceCount,
        uint32_t startIndex, uint32_t vertexOffset, uint32_t startInstance) override;

    void Barrier(uint32_t srcStage, uint32_t dstStage,
        std::vector<BufferBarrier> bufBarriers, std::vector<TextureBarrier> texBarriers) override;

    void CopyToBuffer(Buffer* dst, uint64_t size, void* data) override;
    void CopyToTexture(Texture* dst, uint64_t size, void* data, TextureSubresourceLayers subresource) override;

    void Flush() override;

    void AddWaitSemaphore(VkSemaphore waitSemaphore, uint64_t value = 1);
    void AddSignalSemaphore(VkSemaphore signalSemaphore, uint64_t value = 1);

    void ReleaseResource(ReleaseResourceWrapper* releaseResourceWrapper);
    void EndFrame();

    uint32_t GetQueueFamilyIndex() const { return m_QueueIndex; }
    VkQueue GetVkQueue() const { return m_Queue; }

private:
    void AcquireCommandBuffer();
    void SubmitCommandBuffer();

    void MarkResourcesDirty();
    void BoundDirtyResources();

    void BeginRendering();
    void EndRendering();

    void ClearDirtyAttachmentsIfInsideRendering();

private:
    VulkanDevice* m_Device;
    VkQueue m_Queue = nullptr;
    uint32_t m_QueueIndex = 0;

    ReleaseManager m_ReleaseManager;

    std::unique_ptr<CommandBufferPool> m_CommandBufferPool = nullptr;
    VkCommandBuffer m_CommandBuffer = nullptr;
    uint64_t m_CommandBufferNumber = 0;

    VulkanFence* m_Fence = nullptr;
    std::vector<VkSemaphore> m_WaitSemaphores;
    std::vector<uint64_t> m_WaitSemaphoresValues;
    std::vector<VkSemaphore> m_SignalSemaphores;
    std::vector<uint64_t> m_SignalSemaphoresValues;

    std::unique_ptr<DescriptorManager> m_DescriptorManager = nullptr;

    VulkanGraphicsPipelineState* m_GraphicsPipeline = nullptr;
    bool m_GraphicsPipelineBound = false;
    Viewport m_Viewport;
    bool m_ViewportBound = false;
    Scissor m_Scissor;
    bool m_ScissorBound = false;
    VulkanBuffer* m_VertexBuffer = nullptr;
    bool m_VertexBufferBound = false;
    VulkanBuffer* m_IndexBuffer = nullptr;
    bool m_IndexBufferBound = false;

    std::vector<VulkanRenderTargetView*> m_RTVs;
    VkClearColorValue m_RTVsClearValue;
    bool m_ShouldClearRTVs = false;
    VulkanDepthStencilView* m_DSV = nullptr;
    VkClearDepthStencilValue m_DSVClearValue;
    bool m_ShouldClearDSV = false;
    bool m_InsideRendering = false;

};

} // vk

#endif //DERANGED_RHI_VULKANCOMMANDQUEUE_H
