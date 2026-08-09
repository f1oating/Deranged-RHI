//
// Created by alan on 09/08/2026.
//

#ifndef DERANGED_RHI_FENCE_H
#define DERANGED_RHI_FENCE_H

#include <cstdint>

class Fence {
public:
    virtual ~Fence() = default;

    virtual uint64_t GetCompletedValue() = 0;

    virtual void Wait(uint64_t value) = 0;

};

#endif //DERANGED_RHI_FENCE_H
