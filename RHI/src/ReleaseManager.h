//
// Created by alan on 10/08/2026.
//

#ifndef DERANGED_RHI_RELEASEMANAGER_H
#define DERANGED_RHI_RELEASEMANAGER_H

#include <atomic>
#include <cstdint>
#include <deque>

struct ReleaseResourceBase {
    virtual ~ReleaseResourceBase() = default;

    virtual void Destroy() = 0;

};

class ReleaseResourceWrapper {
public:
    ReleaseResourceWrapper(ReleaseResourceBase* res, uint32_t refCount = 1)
        : m_RefCount(refCount), m_Res(res) {}

    void Release() {
        m_RefCount--;
        if (m_RefCount <= 0) {
            m_Res->Destroy();
            delete m_Res;
            delete this;
        }
    }

private:
    std::atomic<uint32_t> m_RefCount = 0;
    ReleaseResourceBase* m_Res = nullptr;

};

class ReleaseManager {
public:
    void ReleaseResource(ReleaseResourceWrapper* wrapper, uint64_t cmdValue);

    void DiscardStaleResources(uint64_t cmdValue, uint64_t fenceValue);
    void DiscardResources(uint64_t fenceValue);

    void Clear();

private:
    std::deque<std::pair<ReleaseResourceWrapper*, uint64_t>> m_StaleResources;
    std::deque<std::pair<ReleaseResourceWrapper*, uint64_t>> m_ReleaseResources;

};

#endif //DERANGED_RHI_RELEASEMANAGER_H
