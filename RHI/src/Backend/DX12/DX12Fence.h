//
// Created by alan on 09/08/2026.
//

#ifndef DERANGED_RHI_DX12FENCE_H
#define DERANGED_RHI_DX12FENCE_H

#include "Fence.h"
#include "d3d12.h"

class DX12Device;

class DX12Fence : public Fence {
public:
    DX12Fence(DX12Device* device);
    ~DX12Fence() override;

    uint64_t GetCompletedValue() override;

    void Wait(uint64_t value) override;

    ID3D12Fence* GetDX12Fence() const { return m_Fence; }

private:
    DX12Device* m_Device = nullptr;
    ID3D12Fence* m_Fence = nullptr;

};


#endif //DERANGED_RHI_DX12FENCE_H
