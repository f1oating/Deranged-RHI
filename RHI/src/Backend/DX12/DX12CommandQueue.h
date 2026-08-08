//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_DX12COMMANDQUEUE_H
#define DERANGED_RHI_DX12COMMANDQUEUE_H

#include "CommandQueue.h"
#include <d3d12.h>

class DX12Device;

class DX12CommandQueue : public CommandQueue {
public:
    DX12CommandQueue(DX12Device* device);
    ~DX12CommandQueue() override;

    ID3D12CommandQueue* GetDX12CommandQueue() const { return m_Queue; }

private:
    DX12Device* m_Device = nullptr;
    ID3D12CommandQueue* m_Queue = nullptr;

};

#endif //DERANGED_RHI_DX12COMMANDQUEUE_H
