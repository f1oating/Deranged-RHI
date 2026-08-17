//
// Created by alan on 16/08/2026.
//

#ifndef DERANGED_RHI_VARIABLESIZEALLOCATIONMANAGER_H
#define DERANGED_RHI_VARIABLESIZEALLOCATIONMANAGER_H

#include <cstdint>
#include <map>

using Offset = uint64_t;
using Size = uint64_t;
using FreeBlocksByOffset = std::map<Offset, Size>;
using FreeBlocksBySize = std::map<Size, FreeBlocksByOffset::iterator>;
constexpr uint64_t InvalidOffset = UINT64_MAX;

class VariableSizeAllocationManager {
public:
    VariableSizeAllocationManager(Size size);

    Size Allocate(Size size);
    void Free(Offset offset, Size size);

private:
    void AddNewBlock(Offset offset, Size size);

private:
    Size m_FreeSize = 0;
    FreeBlocksByOffset m_FreeBlocksByOffset;
    FreeBlocksBySize m_FreeBlocksBySize;

};

#endif //DERANGED_RHI_VARIABLESIZEALLOCATIONMANAGER_H
