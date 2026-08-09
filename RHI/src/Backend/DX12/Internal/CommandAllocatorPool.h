//
// Created by alan on 09/08/2026.
//

#ifndef DERANGED_RHI_COMMANDALLOCATORPOOL_H
#define DERANGED_RHI_COMMANDALLOCATORPOOL_H

#include <cstdint>
#include <d3d12.h>
#include <deque>

class CommandAllocatorPool {
public:
    void Init(ID3D12Device* device);
    void Shutdown();

    ID3D12CommandAllocator* AcquireCommandAllocator();
    void ReleaseCommandAllocator(ID3D12CommandAllocator* commandAllocator, uint64_t value);

    void Poll(uint64_t value);

private:
    ID3D12Device* m_Device = nullptr;
    std::deque<ID3D12CommandAllocator*> m_AcquireQueue;
    std::deque<std::pair<ID3D12CommandAllocator*, uint64_t>> m_ReleaseQueue;

};

#endif //DERANGED_RHI_COMMANDALLOCATORPOOL_H
