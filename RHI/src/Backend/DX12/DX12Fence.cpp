//
// Created by alan on 09/08/2026.
//

#include "DX12Fence.h"

#include "DX12Device.h"

DX12Fence::DX12Fence(DX12Device* device) {
    m_Device = device;

    HRESULT hr = m_Device->GetDX12Device()->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_Fence));
    m_Event = CreateEvent(nullptr, true, true, nullptr);
}

DX12Fence::~DX12Fence() {
    if (m_Event) {
        CloseHandle(m_Event);
    }
    if (m_Fence) {
        m_Fence->Release();
    }
}

uint64_t DX12Fence::GetCompletedValue() {
    return m_Fence->GetCompletedValue();
}

void DX12Fence::Wait(uint64_t value) {
    m_Fence->SetEventOnCompletion(value, m_Event);
    WaitForSingleObject(m_Event, INFINITE);
}