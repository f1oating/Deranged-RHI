//
// Created by alan on 13/08/2026.
//

#include "Backend/DX12/Internal/RingBuffer.h"

namespace dx {

void RingBuffer::Init(ID3D12Device10* device, uint64_t size) {
    m_Device = device;
    m_Size = size;

    D3D12_HEAP_PROPERTIES heapProps = {
        .Type = D3D12_HEAP_TYPE_UPLOAD
    };

    D3D12_HEAP_DESC heapDesc = {
        .SizeInBytes = m_Size,
        .Properties =  heapProps,
        .Flags = D3D12_HEAP_FLAG_NONE
    };

    m_Device->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_Heap));
}

uint64_t RingBuffer::Allocate(uint64_t size) {
    uint64_t alignedSize = AlignUp(size, (uint64_t)256);

    if (m_Head + alignedSize > 2560) {
        m_Head = 0;
    }

    uint64_t offset = m_Head;
    m_Head += alignedSize;

    return offset;
}

void RingBuffer::Shutdown() {
    if (m_Heap) {
        m_Heap->Release();
    }
}

} // dx
