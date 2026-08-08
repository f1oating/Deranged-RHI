//
// Created by alan on 08/08/2026.
//

#include "Device.h"
#include "Backend/Vulkan/VulkanDevice.h"

Device* Device::Create() {
    return new VulkanDevice();
}
