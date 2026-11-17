//
// Created by alan on 16/11/2026.
//

#ifndef DERANGED_RHI_RENDERGRAPH_H
#define DERANGED_RHI_RENDERGRAPH_H

#include <cstdint>
#include <vector>
#include "Device.h"
#include <string>

using RGHandle = std::uint64_t;

class RenderGraph;

struct Pass {
    std::string Name;
    void(*Callback)(RenderGraph* renderGraph);
    std::vector<RGHandle> input;
    std::vector<RGHandle> output;
};

class RenderGraph {
public:
    RenderGraph(Device* device);
    ~RenderGraph();

    RGHandle CreateTexture(TextureDesc desc);
    Texture* GetTexture(RGHandle handle);

    void AddPass(std::string name, void(*callback)(RenderGraph* renderGraph),
        std::vector<RGHandle> input, std::vector<RGHandle> output);

    void Compile();
    void Execute();

private:
    void ProccessPass(uint32_t index, std::vector<std::vector<uint32_t>>& adj, std::vector<bool>& visited,
        std::vector<bool>& onStack, std::vector<uint32_t>& sorted);

private:
    Device* m_Device = nullptr;

    std::vector<Texture*> m_Textures;
    std::vector<Pass> m_Passes;
    std::vector<uint32_t> m_SortedPassIndexes;

};

#endif //DERANGED_RHI_RENDERGRAPH_H
