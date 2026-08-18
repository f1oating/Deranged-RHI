//
// Created by alan on 09/08/2026.
//

#include "Backend/DX12/Internal/CommandAllocatorPool.h"

namespace dx {

void CommandAllocatorPool::Init(ID3D12Device* device) {
    m_Device = device;
}

void CommandAllocatorPool::Shutdown() {
    Poll(UINT64_MAX);
    while (!m_AcquireQueue.empty()) {
        m_AcquireQueue.front()->Release();
        m_AcquireQueue.pop_front();
    }
}

ID3D12CommandAllocator* CommandAllocatorPool::AcquireCommandAllocator() {
    if (!m_AcquireQueue.empty()) {
        ID3D12CommandAllocator* commandAllocator = m_AcquireQueue.front();
        m_AcquireQueue.pop_front();
        return commandAllocator;
    }

    ID3D12CommandAllocator* commandAllocator;
    HRESULT hr = m_Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator));
    return commandAllocator;
}

void CommandAllocatorPool::ReleaseCommandAllocator(ID3D12CommandAllocator* commandAllocator, uint64_t value) {
    m_ReleaseQueue.emplace_back(commandAllocator, value);
}

void CommandAllocatorPool::Poll(uint64_t value) {
    while (!m_ReleaseQueue.empty()) {
        if (m_ReleaseQueue.front().second <= value) {
            m_AcquireQueue.push_back(m_ReleaseQueue.front().first);
            m_ReleaseQueue.pop_front();
            continue;
        }
        break;
    }
}

} // dx
