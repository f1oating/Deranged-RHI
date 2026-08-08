//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_SWAPCHAIN_H
#define DERANGED_RHI_SWAPCHAIN_H

class Swapchain {
public:
    virtual ~Swapchain() = default;

    virtual void UpdateWindow() = 0;
    virtual bool WindowShouldClose() = 0;
    virtual void Present() = 0;

};

#endif //DERANGED_RHI_SWAPCHAIN_H
