//
// Created by alan on 17/08/2026.
//

#include "Backend/DX12/Internal/DescriptorHeapAllocator.h"

namespace dx {

DescriptorHeapAllocation::DescriptorHeapAllocation()
    : m_Heap(nullptr), m_CPUHandle(), m_GPUHandle(), m_NumHandles(0), m_DescriptorSize(0) {}

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

void DescriptorHeapAllocator::Init(ID3D12Device4* device,D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors){
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

void DescriptorHeapAllocator::Shutdown() {
    if (m_Heap) {
        m_Heap->Release();
    }
}

DescriptorHeapAllocation DescriptorHeapAllocator::Allocate(uint32_t numHandles) {
    uint64_t offset = m_VariableSizeAllocationManager.Allocate(numHandles);

    D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle{};
    D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle{};

    if (m_ShaderVisible) {
        gpuHandle = m_Heap->GetGPUDescriptorHandleForHeapStart();
        gpuHandle.ptr += offset * m_DescriptorSize;
    } else {
        cpuHandle = m_Heap->GetCPUDescriptorHandleForHeapStart();
        cpuHandle.ptr += offset * m_DescriptorSize;
    }

    return DescriptorHeapAllocation(m_Heap, cpuHandle, gpuHandle, numHandles, m_DescriptorSize);
}

void DescriptorHeapAllocator::Free(DescriptorHeapAllocation allocation) {
    uint64_t offset;
    if (allocation.IsShaderVisible()) {
        offset = (allocation.GetGPUHandle(0).ptr - m_Heap->GetGPUDescriptorHandleForHeapStart().ptr) / m_DescriptorSize;
    } else {
        offset = (allocation.GetCPUHandle(0).ptr - m_Heap->GetCPUDescriptorHandleForHeapStart().ptr) / m_DescriptorSize;
    }
    m_VariableSizeAllocationManager.Free(offset, allocation.GetNumHandles());
}

} // dx