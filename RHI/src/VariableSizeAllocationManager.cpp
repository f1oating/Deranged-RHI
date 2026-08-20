//
// Created by alan on 16/08/2026.
//

#include "VariableSizeAllocationManager.h"

void VariableSizeAllocationManager::Init(Size size) {
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
    auto nextOffsetBlockIt = m_FreeBlocksByOffset.upper_bound(offset);
    auto prevOffsetBlockIt = nextOffsetBlockIt;

    if (prevOffsetBlockIt != m_FreeBlocksByOffset.begin()) {
        --prevOffsetBlockIt;
    } else {
        prevOffsetBlockIt = m_FreeBlocksByOffset.end();
    }

    Offset newOffset;
    Size newSize;

    if (prevOffsetBlockIt != m_FreeBlocksByOffset.end() && offset == prevOffsetBlockIt->first + prevOffsetBlockIt->second.BlockSize) {
        newSize = prevOffsetBlockIt->second.BlockSize + size;
        newOffset = prevOffsetBlockIt->first;

        if (nextOffsetBlockIt != m_FreeBlocksByOffset.end() && offset + size == nextOffsetBlockIt->first) {
            newSize += nextOffsetBlockIt->second.BlockSize;

            m_FreeBlocksBySize.erase(prevOffsetBlockIt->second.OrderBySize);
            m_FreeBlocksBySize.erase(nextOffsetBlockIt->second.OrderBySize);
            m_FreeBlocksByOffset.erase(prevOffsetBlockIt);
            m_FreeBlocksByOffset.erase(nextOffsetBlockIt);
        } else {
            m_FreeBlocksBySize.erase(prevOffsetBlockIt->second.OrderBySize);
            m_FreeBlocksByOffset.erase(prevOffsetBlockIt);
        }
    } else if (nextOffsetBlockIt != m_FreeBlocksByOffset.end() && offset + size == nextOffsetBlockIt->first) {
        newSize = size + nextOffsetBlockIt->second.BlockSize;
        newOffset = offset;
        m_FreeBlocksBySize.erase(nextOffsetBlockIt->second.OrderBySize);
        m_FreeBlocksByOffset.erase(nextOffsetBlockIt);
    } else {
        newSize = size;
        newOffset = offset;
    }

    AddNewBlock(newOffset, newSize);

    m_FreeSize += size;
}

void VariableSizeAllocationManager::AddNewBlock(Offset offset, Size size) {
    auto newBlockByOffsetIt = m_FreeBlocksByOffset.emplace(offset, size);
    auto newBlockBySizeIt = m_FreeBlocksBySize.emplace(size, newBlockByOffsetIt.first);
    newBlockByOffsetIt.first->second.OrderBySize = newBlockBySizeIt.first;
}
