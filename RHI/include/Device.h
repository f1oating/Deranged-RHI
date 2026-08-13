//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_DEVICE_H
#define DERANGED_RHI_DEVICE_H

#include "CommandQueue.h"
#include "Pipeline.h"
#include "Swapchain.h"
#include "Resource.h"

class Device {
public:
    virtual ~Device() = default;

    static Device* Create();

    virtual void EndFrame() = 0;

    virtual CommandQueue* GetCommandQueue() = 0;
    virtual Swapchain* CreateSwapchain() = 0;
    virtual GraphicsPipelineState* CreateGraphicsPipelineState(GraphicsPipelineDesc desc) = 0;
    virtual Texture* CreateTexture(TextureDesc desc) = 0;
    virtual TextureView* CreateTextureView(TextureViewDesc desc) = 0;

};

#endif //DERANGED_RHI_DEVICE_H
