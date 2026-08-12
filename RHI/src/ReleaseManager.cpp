//
// Created by alan on 10/08/2026.
//

#include "ReleaseManager.h"

#include <cstdio>

void ReleaseManager::ReleaseResource(ReleaseResourceWrapper* wrapper, uint64_t cmdValue) {
    m_ReleaseResources.emplace_back(wrapper, cmdValue);
}

void ReleaseManager::DiscardStaleResources(uint64_t cmdValue, uint64_t fenceValue) {
    while (!m_StaleResources.empty()) {
        if (m_StaleResources.front().second <= cmdValue) {
            m_ReleaseResources.emplace_back(m_StaleResources.front().first, fenceValue);
            m_StaleResources.pop_front();
            continue;
        }
        break;
    }
}

void ReleaseManager::DiscardResources(uint64_t fenceValue) {
    while (!m_ReleaseResources.empty()) {
        if (m_ReleaseResources.front().second <= fenceValue) {
            m_ReleaseResources.front().first->Release();
            m_ReleaseResources.pop_front();
            continue;
        }
        break;
    }
}

void ReleaseManager::Clear() {
    DiscardStaleResources(UINT64_MAX, UINT64_MAX);
    DiscardResources(UINT64_MAX);
}