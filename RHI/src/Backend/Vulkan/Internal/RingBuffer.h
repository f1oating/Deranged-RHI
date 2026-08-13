//
// Created by alan on 13/08/2026.
//

#ifndef DERANGED_RHI_RINGBUFFER_H
#define DERANGED_RHI_RINGBUFFER_H

#include <volk.h>

template<typename T> inline T AlignUp(T size, T alignment) {
    return (size + alignment - (T)1) & ~(alignment - (T)1);
}

class RingBuffer {
public:
    void Init(VkDevice device, VkPhysicalDevice physDevice);
    void Shutdown();

    uint64_t Allocate(uint64_t size);

    void SetTail(uint64_t tail) { m_Tail = tail; }
    VkBuffer GetBuffer() const { return m_Buffer; }

private:
    VkDevice m_Device = nullptr;
    VkPhysicalDevice m_PhysDevice = nullptr;
    VkBuffer m_Buffer = nullptr;
    VkDeviceMemory m_Memory = nullptr;
    uint64_t m_Head = 0;
    uint64_t m_Tail = 0;

};

#endif //DERANGED_RHI_RINGBUFFER_H
