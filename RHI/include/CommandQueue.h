//
// Created by alan on 08/08/2026.
//

#ifndef DERANGED_RHI_COMMANDQUEUE_H
#define DERANGED_RHI_COMMANDQUEUE_H

#include "Fence.h"

class CommandQueue {
public:
    virtual ~CommandQueue() = default;

    virtual void Wait(Fence* fence, uint64_t value) = 0;
    virtual void Signal(Fence* fence, uint64_t value) = 0;

    virtual void Flush() = 0;

};

#endif //DERANGED_RHI_COMMANDQUEUE_H
