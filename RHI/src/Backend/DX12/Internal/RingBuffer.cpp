//
// Created by alan on 13/08/2026.
//

#include "Backend/DX12/Internal/RingBuffer.h"

namespace dx {

void RingBuffer::Init(ID3D12Device10* device) {
    m_Device = device;

    D3D12_HEAP_PROPERTIES heapProps = {
        .Type = D3D12_HEAP_TYPE_UPLOAD
    };

    D3D12_RESOURCE_DESC resourceDesc = {
        .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
        .Width = 2560,
        .Height = 1,
        .DepthOrArraySize = 1,
        .MipLevels = 1,
        .SampleDesc = { 1, 0 },
        .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR
    };

    m_Device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &resourceDesc,
        D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&m_Resource));
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
    if (m_Resource) {
        m_Resource->Release();
    }
}

} // dx
