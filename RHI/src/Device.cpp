//
// Created by alan on 08/08/2026.
//

#include "Device.h"

#ifdef WIN32
    #include "Backend/DX12/DX12Device.h"
#else
    #include "Backend/Vulkan/VulkanDevice.h"
#endif

Device* Device::Create() {
#ifdef WIN32
    return new DX12Device();
#else
    return new VulkanDevice();
#endif
}
