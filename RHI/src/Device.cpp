//
// Created by alan on 08/08/2026.
//

#include "Device.h"

#ifdef WIN32
    #include "Backend/DX12/DX12Device.h"
#else
    #include "Backend/Vulkan/VulkanDevice.h"
#endif

Device* Device::Create(DeviceDesc desc) {
#ifdef WIN32
    return new dx::DX12Device(desc);
#else
    return new vk::VulkanDevice(desc);
#endif
}
