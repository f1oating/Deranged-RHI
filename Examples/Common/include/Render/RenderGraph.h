//
// Created by alan on 16/11/2026.
//

#ifndef DERANGED_RHI_RENDERGRAPH_H
#define DERANGED_RHI_RENDERGRAPH_H

#include <cstdint>
#include <vector>
#include "Device.h"
#include <string>
#include <functional>

using RGHandle = std::uint64_t;

class RenderGraph;

struct Pass {
    std::string Name;
    std::function<void(RenderGraph* renderGraph)> Callback;
    std::vector<RGHandle> Input;
    std::vector<RGHandle> Output;
};

class RenderGraph {
public:
    RenderGraph(Device* device);
    ~RenderGraph();

    RGHandle CreateTexture(TextureDesc desc);
    Texture* GetTexture(RGHandle handle);

    void AddPass(std::string name, std::function<void(RenderGraph* renderGraph)> callback,
        std::vector<RGHandle> input, std::vector<RGHandle> output);

    void Compile();
    void Execute();

private:
    bool ProccessPass(uint32_t index, std::vector<std::vector<uint32_t>>& adj, std::vector<bool>& visited,
        std::vector<bool>& onStack, std::vector<uint32_t>& sorted);

private:
    Device* m_Device = nullptr;

    std::vector<Texture*> m_Textures;
    std::vector<Pass> m_Passes;
    std::vector<uint32_t> m_SortedPassIndexes;

};

#endif //DERANGED_RHI_RENDERGRAPH_H
