//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_DEVICE_H
#define DERANGED_RHI_DEVICE_H

#include "Swapchain.h"

class Device {
public:
    virtual ~Device() = default;

    static Device* Create();

    virtual Swapchain* CreateSwapchain() = 0;

};

#endif //DERANGED_RHI_DEVICE_H
