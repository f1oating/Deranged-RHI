//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_SWAPCHAIN_H
#define DERANGED_RHI_SWAPCHAIN_H

#include "Resource.h"

class Swapchain {
public:
    virtual ~Swapchain() = default;

    virtual Texture* GetCurrentBackBuffer() = 0;

    virtual void UpdateWindow() = 0;
    virtual bool WindowShouldClose() = 0;
    virtual void Present() = 0;

};

#endif //DERANGED_RHI_SWAPCHAIN_H
