//
// Created by alan on 08/08/2026.
//

#include "Backend/DX12/DX12CommandQueue.h"
#include "Backend/DX12/DX12Device.h"

DX12CommandQueue::DX12CommandQueue(DX12Device* device) {
    m_Device = device;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    HRESULT hr = m_Device->GetDX12Device()->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_Queue));
}

DX12CommandQueue::~DX12CommandQueue() {
    if (m_Queue) {
        m_Queue->Release();
    }
}
