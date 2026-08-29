//
// Created by alan on 17/08/2026.
//

#include "Backend/DX12/Internal/DescriptorHeap.h"
#include <iostream>

namespace dx {

DescriptorHeapAllocation::DescriptorHeapAllocation()
    : m_Heap(nullptr), m_CPUHandle(0), m_GPUHandle(0), m_NumHandles(0), m_DescriptorSize(0) {}

DescriptorHeapAllocation::DescriptorHeapAllocation(ID3D12DescriptorHeap* heap, D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle,
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle, uint32_t handlesCount, uint32_t descriptorSize) {
    m_Heap = heap;
    m_CPUHandle = cpuHandle;
    m_GPUHandle = gpuHandle;
    m_NumHandles = handlesCount;
    m_DescriptorSize = descriptorSize;
}

D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeapAllocation::GetCPUHandle(uint32_t offset) const {
    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle = m_CPUHandle;
    if (offset != 0) {
        cpuHandle.ptr += m_DescriptorSize * offset;
    }
    return cpuHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeapAllocation::GetGPUHandle(uint32_t offset) const {
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = m_GPUHandle;
    if (offset != 0) {
        gpuHandle.ptr += m_DescriptorSize * offset;
    }
    return gpuHandle;
}

void DescriptorHeap::Init(ID3D12Device10* device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors){
    m_Device = device;
    m_NumDescriptors = numDescriptors;

    m_VariableSizeAllocationManager.Init(numDescriptors);

    m_ShaderVisible = !(type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV || type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    D3D12_DESCRIPTOR_HEAP_FLAGS flags = type ?
        D3D12_DESCRIPTOR_HEAP_FLAG_NONE : D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;

    D3D12_DESCRIPTOR_HEAP_DESC desc = {
        .Type = type,
        .NumDescriptors = numDescriptors,
        .Flags = flags,
    };
    m_Device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&m_Heap));
    m_DescriptorSize = m_Device->GetDescriptorHandleIncrementSize(type);
}

void DescriptorHeap::Shutdown() {
    if (m_Heap) {
        m_Heap->Release();
    }
}

DescriptorHeapAllocation DescriptorHeap::Allocate(uint32_t numHandles) {
    uint64_t offset = m_VariableSizeAllocationManager.Allocate(numHandles);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{};

    cpuHandle = m_Heap->GetCPUDescriptorHandleForHeapStart();
    cpuHandle.ptr += offset * m_DescriptorSize;

    if (m_ShaderVisible) {
        gpuHandle = m_Heap->GetGPUDescriptorHandleForHeapStart();
        gpuHandle.ptr += offset * m_DescriptorSize;
    }

    return DescriptorHeapAllocation(m_Heap, cpuHandle, gpuHandle, numHandles, m_DescriptorSize);
}

void DescriptorHeap::Free(DescriptorHeapAllocation allocation) {
    uint64_t offset;
    if (allocation.IsShaderVisible()) {
        offset = (allocation.GetGPUHandle(0).ptr - m_Heap->GetGPUDescriptorHandleForHeapStart().ptr) / m_DescriptorSize;
    } else {
        offset = (allocation.GetCPUHandle(0).ptr - m_Heap->GetCPUDescriptorHandleForHeapStart().ptr) / m_DescriptorSize;
    }
    m_VariableSizeAllocationManager.Free(offset, allocation.GetNumHandles());
}

void DescriptorsStateManager::Init(ID3D12Device10* device) {
    m_Device = device;
    m_Heap.Init(m_Device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128);
}

void DescriptorsStateManager::Shutdown() {
    m_Heap.Shutdown();
}

void DescriptorsStateManager::SetState(std::unordered_map<uint32_t, Descriptor> descriptorsStruct) {
    m_DescriptorsState = descriptorsStruct;
}

void DescriptorsStateManager::SetCBV(uint32_t offset, D3D12_CONSTANT_BUFFER_VIEW_DESC cbvViewDesc) {
    m_DescriptorsState.at(offset).CBVDesc = cbvViewDesc;
}

void DescriptorsStateManager::SetSRV(uint32_t offset, ID3D12Resource* resource, D3D12_SHADER_RESOURCE_VIEW_DESC srvViewDesc) {
    m_DescriptorsState.at(offset).SRVDesc = srvViewDesc;
    m_DescriptorsState.at(offset).Resource = resource;
}

DescriptorHeapAllocation DescriptorsStateManager::WriteAndAllocate(uint64_t frame) {
    DescriptorHeapAllocation allocation = m_Heap.Allocate(m_DescriptorsState.size());

    for (auto pair : m_DescriptorsState) {
        if (pair.second.Type == DescriptorType::ConstantBuffer) {
            m_Device->CreateConstantBufferView(&pair.second.CBVDesc, allocation.GetCPUHandle(pair.first));
        } else {
            m_Device->CreateShaderResourceView(pair.second.Resource, &pair.second.SRVDesc,allocation.GetCPUHandle(pair.first));
        }
    }

    m_Allocations.emplace_back(frame, allocation);
    return allocation;
}

void DescriptorsStateManager::Clear() {
    m_DescriptorsState.clear();
}

void DescriptorsStateManager::FreeFrames(uint64_t frame) {
    while (!m_Allocations.empty()) {
        if (m_Allocations.front().first <= frame) {
            DescriptorHeapAllocation allocation = m_Allocations.front().second;
            m_Heap.Free(allocation);
            m_Allocations.pop_front();
            continue;
        }
        break;
    }
}

} // dx