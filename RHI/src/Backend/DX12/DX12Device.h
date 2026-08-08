//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_DX12DEVICE_H
#define DERANGED_RHI_DX12DEVICE_H

#include "Device.h"
#include <dxgi1_3.h>
#include <d3d12.h>
#include "Backend/DX12/DX12CommandQueue.h"

class DX12Device : public Device {
public:
    DX12Device();
    ~DX12Device() override;

    Swapchain* CreateSwapchain() override;

    IDXGIFactory3* GetDXGIFactory() const { return m_Factory;}
    ID3D12Device* GetDX12Device() const { return m_Device; }
    DX12CommandQueue* GetCommandQueue() const { return m_CommandQueue; }

private:
    IDXGIFactory3* m_Factory = nullptr;
    ID3D12Device* m_Device = nullptr;
    DX12CommandQueue* m_CommandQueue = nullptr;

};

#endif //DERANGED_RHI_DX12DEVICE_H
