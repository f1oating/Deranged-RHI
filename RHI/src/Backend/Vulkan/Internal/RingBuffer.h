//
// Created by alan on 13/08/2026.
//

#ifndef DERANGED_RHI_RINGBUFFER_H
#define DERANGED_RHI_RINGBUFFER_H

#include <volk.h>

namespace vk {

template<typename T> inline T AlignUp(T size, T alignment) {
    return (size + alignment - (T)1) & ~(alignment - (T)1);
}

class RingBuffer {
public:
    void Init(VkDevice device, VkPhysicalDevice physDevice, uint64_t size = 512 * 512 * 4);
    void Shutdown();

    uint64_t Allocate(uint64_t size);

    void SetTail(uint64_t tail) { m_Tail = tail; }
    VkDeviceMemory GetMemory() const { return m_Memory; }
    uint64_t GetHead() const { return m_Head; }
    void* GetMapped() const { return m_Mapped; }

private:
    VkDevice m_Device = nullptr;
    VkPhysicalDevice m_PhysDevice = nullptr;
    VkDeviceMemory m_Memory = nullptr;
    uint64_t m_Size = 0;
    uint64_t m_Head = 0;
    uint64_t m_Tail = 0;
    void* m_Mapped = nullptr;

};

} // vk

#endif //DERANGED_RHI_RINGBUFFER_H
