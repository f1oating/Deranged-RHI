//
// Created by alan on 16/08/2026.
//

#include "VariableSizeAllocationManager.h"

VariableSizeAllocationManager::VariableSizeAllocationManager(Size size) {
    m_FreeSize = size;
    AddNewBlock(0, size);
}

Size VariableSizeAllocationManager::Allocate(Size size) {
    if (m_FreeSize < size) {
        return InvalidOffset;
    }

    auto smallestSizeBlockIt = m_FreeBlocksBySize.lower_bound(size);
    if (smallestSizeBlockIt == m_FreeBlocksBySize.end()) {
        return InvalidOffset;
    }

    auto offsetBlockIt = smallestSizeBlockIt->second;
    Offset offset = offsetBlockIt->first;
    Offset newOffset = offset + size;
    Size newSize = smallestSizeBlockIt->first - size;
    m_FreeBlocksBySize.erase(smallestSizeBlockIt);
    m_FreeBlocksByOffset.erase(offsetBlockIt);
    if (newSize > 0) {
        AddNewBlock(newOffset, newSize);
    }

    m_FreeSize = newSize;
    return offset;
}

void VariableSizeAllocationManager::Free(Offset offset, Size size) {
    auto nextOffsetBlockIt = m_FreeBlocksByOffset.upper_bound(size);
    auto prevOffsetBlockIt = nextOffsetBlockIt;

    if (nextOffsetBlockIt != m_FreeBlocksByOffset.begin()) {
        --prevOffsetBlockIt;
    } else {
        prevOffsetBlockIt = m_FreeBlocksByOffset.end();
    }

    Offset newOffset;
    Size newSize;

    if (prevOffsetBlockIt != m_FreeBlocksByOffset.end() && offset == prevOffsetBlockIt->first + prevOffsetBlockIt->second) {
        newSize = prevOffsetBlockIt->second + size;
        newOffset = prevOffsetBlockIt->first;

        if (nextOffsetBlockIt != m_FreeBlocksByOffset.end() && offset + size == nextOffsetBlockIt->first) {
            newSize += nextOffsetBlockIt->second;

            m_FreeBlocksBySize.erase(m_FreeBlocksBySize.find(prevOffsetBlockIt->second));
            m_FreeBlocksBySize.erase(m_FreeBlocksBySize.find(nextOffsetBlockIt->second));
            m_FreeBlocksByOffset.erase(prevOffsetBlockIt);
            m_FreeBlocksByOffset.erase(nextOffsetBlockIt);
        } else {
            m_FreeBlocksBySize.erase(m_FreeBlocksBySize.find(prevOffsetBlockIt->second));
            m_FreeBlocksByOffset.erase(prevOffsetBlockIt);
        }
    } else if (nextOffsetBlockIt != m_FreeBlocksByOffset.end() && offset + size == nextOffsetBlockIt->first) {
        newSize = size + nextOffsetBlockIt->second;
        newOffset = offset;
        m_FreeBlocksBySize.erase(m_FreeBlocksBySize.find(nextOffsetBlockIt->second));
        m_FreeBlocksByOffset.erase(nextOffsetBlockIt);
    } else {
        newSize = size;
        newOffset = offset;
    }

    AddNewBlock(newOffset, newSize);

    m_FreeSize += size;
}

void VariableSizeAllocationManager::AddNewBlock(Offset offset, Size size) {
    auto newBlockIt = m_FreeBlocksByOffset.emplace(offset, size);
    m_FreeBlocksBySize.emplace(size, newBlockIt.first);
}
