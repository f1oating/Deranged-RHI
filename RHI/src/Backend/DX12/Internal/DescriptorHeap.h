//
// Created by alan on 17/08/2026.
//

#ifndef DERANGED_RHI_DESCRIPTORHEAPALLOCATOR_H
#define DERANGED_RHI_DESCRIPTORHEAPALLOCATOR_H

#include <cstdint>
#include <d3d12.h>
#include <deque>
#include "VariableSizeAllocationManager.h"
#include <unordered_map>

namespace dx {

class DescriptorHeapAllocation {
public:
    DescriptorHeapAllocation();
    DescriptorHeapAllocation(ID3D12DescriptorHeap* heap, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
        D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle, uint32_t handlesCount, uint32_t descriptorSize);

    D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(uint32_t offset) const;
    D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(uint32_t offset) const;

    ID3D12DescriptorHeap* GetHeap() const { return m_Heap; }
    uint64_t GetNumHandles() const { return m_NumHandles; }

    bool IsNull() const { return m_CPUHandle.ptr == 0; }
    bool IsShaderVisible() const { return m_GPUHandle.ptr != 0; }
    uint32_t GetDescriptorSize() const { return m_DescriptorSize; }

private:
    ID3D12DescriptorHeap* m_Heap = nullptr;
    uint64_t m_NumHandles = 0;
    D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle = {};
    D3D12_GPU_DESCRIPTOR_HANDLE m_GPUHandle = {};
    uint32_t m_DescriptorSize = 0;

};

class DescriptorHeap {
public:
    void Init(ID3D12Device10* device, D3D12_DESCRIPTOR_HEAP_TYPE type,
        uint32_t numDescriptors);
    void Shutdown();

    DescriptorHeapAllocation Allocate(uint32_t numHandles);
    void Free(DescriptorHeapAllocation allocation);

    ID3D12DescriptorHeap* GetDX12Heap() const { return m_Heap; }

private:
    ID3D12Device10* m_Device = nullptr;
    ID3D12DescriptorHeap* m_Heap = nullptr;
    uint64_t m_NumDescriptors = 0;
    uint32_t m_DescriptorSize = 0;
    VariableSizeAllocationManager m_VariableSizeAllocationManager;
    bool m_ShaderVisible = false;

};

enum class DescriptorType {
    ConstantBuffer,
    ShaderResource
};

struct Descriptor {
    DescriptorType Type;
    D3D12_CONSTANT_BUFFER_VIEW_DESC CBVDesc;
    D3D12_SHADER_RESOURCE_VIEW_DESC SRVDesc;
    ID3D12Resource* Resource;
};

class DescriptorsStateManager {
public:
    void Init(ID3D12Device10* device);
    void Shutdown();

    void SetState(std::unordered_map<uint32_t, Descriptor> descriptorsState);

    void SetCBV(uint32_t offset, D3D12_CONSTANT_BUFFER_VIEW_DESC cbvViewDesc);
    void SetSRV(uint32_t offset, ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC srvViewDesc);

    DescriptorHeapAllocation WriteAndAllocate(uint64_t frame);

    void Clear();

    void FreeFrames(uint64_t frame);

    ID3D12DescriptorHeap* GetDX12Heap() { return m_Heap.GetDX12Heap(); }

private:
    ID3D12Device10* m_Device = nullptr;
    DescriptorHeap m_Heap;
    std::deque<std::pair<uint64_t, DescriptorHeapAllocation>> m_Allocations;
    std::unordered_map<uint32_t, Descriptor> m_DescriptorsState;

};

} // dx

#endif //DERANGED_RHI_DESCRIPTORHEAPALLOCATOR_H
