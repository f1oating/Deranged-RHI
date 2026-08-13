//
// Created by alan on 10/08/2026.
//

#include "ReleaseManager.h"

#include <cstdio>

void ReleaseManager::ReleaseResource(ReleaseResourceWrapper* wrapper) {
    m_StaleResources.push_back(wrapper);
}

void ReleaseManager::DiscardStaleResources(uint64_t value) {
    while (!m_StaleResources.empty()) {
        m_ReleaseResources.emplace_back(m_StaleResources.front(), value);
        m_StaleResources.pop_front();
    }
}

void ReleaseManager::DiscardResources(uint64_t value) {
    while (!m_ReleaseResources.empty()) {
        if (m_ReleaseResources.front().second <= value) {
            m_ReleaseResources.front().first->Release();
            m_ReleaseResources.pop_front();
            continue;
        }
        break;
    }
}

void ReleaseManager::Clear() {
    DiscardStaleResources(UINT64_MAX);
    DiscardResources(UINT64_MAX);
}