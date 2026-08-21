//
// Created by alan on 13/08/2026.
//

#ifndef DERANGED_RHI_RINGBUFFER_H
#define DERANGED_RHI_RINGBUFFER_H

#include <cstdint>
#include <d3d12.h>

namespace dx {

template<typename T> inline T AlignUp(T val, T alignment)
{
    return (val + alignment - (T)1) & ~(alignment - (T)1);
}

class RingBuffer {
public:
    void Init(ID3D12Device10* device);
    void Shutdown();

    uint64_t Allocate(uint64_t size);

    void SetTail(uint64_t tail) { m_Tail = tail; }

    uint64_t GetHead() const { return m_Head; }
    ID3D12Resource* GetDX12Resource() { return m_Resource; }

private:
    ID3D12Device10* m_Device = nullptr;
    ID3D12Resource* m_Resource = nullptr;
    uint64_t m_Tail = 0;
    uint64_t m_Head = 0;

};

} // dx

#endif //DERANGED_RHI_RINGBUFFER_H
