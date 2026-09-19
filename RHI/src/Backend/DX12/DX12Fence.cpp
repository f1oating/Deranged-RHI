//
// Created by alan on 09/08/2026.
//

#include "DX12Fence.h"
#include <spdlog/spdlog.h>
#include "DX12Device.h"

namespace dx {

DX12Fence::DX12Fence(DX12Device* device) {
    m_Device = device;

    HRESULT hr = m_Device->GetDX12Device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));

    spdlog::info("DX12Fence Created.");
}

DX12Fence::~DX12Fence() {
    if (m_Fence) {
        m_Device->ReleaseResource(new FenceReleaseResource(m_Fence));
    }

    spdlog::info("DX12Fence Destroyed.");
}

uint64_t DX12Fence::GetCompletedValue() {
    return m_Fence->GetCompletedValue();
}

void DX12Fence::Wait(uint64_t value) {
    if (value) {
        m_Fence->SetEventOnCompletion(value, nullptr);
    }
}

} // dx