//
// Created by alan on 08/08/2026.
//

#include "Device.h"
#include "Backend/Vulkan/VulkanDevice.h"
#include "Backend/DX12/DX12Device.h"

Device* Device::Create() {
    return new DX12Device();
}
