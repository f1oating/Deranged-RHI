//
// Created by alan on 16/11/2026.
//

#include "Render/RenderGraph.h"

RenderGraph::RenderGraph(Device* device) {
    m_Device = device;
}

RenderGraph::~RenderGraph() {
    for (auto* texture : m_Textures) {
        delete texture;
    }
}

RGHandle RenderGraph::CreateTexture(TextureDesc desc) {
    m_Textures.push_back(m_Device->CreateTexture(desc));
    return m_Textures.size();
}

Texture* RenderGraph::GetTexture(RGHandle handle) {
    return m_Textures[handle];
}

void RenderGraph::AddPass(std::string name, std::function<void(RenderGraph* renderGraph)> callback,
    std::vector<RGHandle> input, std::vector<RGHandle> output) {
    m_Passes.push_back({ name, callback, input, output });
}

void RenderGraph::Compile() {
    int size = m_Passes.size();
    m_SortedPassIndexes.reserve(size);

    std::vector<std::vector<uint32_t>> adj(size);
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (j == i) continue;
            for (int k = 0; k < m_Passes[i].Input.size(); k++) {
                RGHandle handle = m_Passes[i].Input[k];
                bool found = false;

                for (int l = 0; l < m_Passes[j].Output.size(); l++) {
                    if (m_Passes[j].Output[l] == handle) {
                        adj[i].push_back(j);
                        found = true;
                    }

                    if (found) break;
                }

                if (found) break;
            }
        }
    }

    std::vector visited(size, false);
    std::vector onStack(size, false);

    for (int i = 0; i < adj.size(); i++) {
        if (!ProccessPass(i, adj, visited, onStack, m_SortedPassIndexes)) {
            m_SortedPassIndexes.clear();
            break;
        }
    }
}

void RenderGraph::Execute() {
    for (auto passIndex : m_SortedPassIndexes) {
        m_Passes[passIndex].Callback(this);
    }
}

bool RenderGraph::ProccessPass(uint32_t index, std::vector<std::vector<uint32_t>>& adj, std::vector<bool>& visited,
        std::vector<bool>& onStack, std::vector<uint32_t>& sorted) {
    if (onStack[index]) return false;
    if (visited[index]) return true;

    visited[index] = true;
    onStack[index] = true;

    for (int i = 0; i < adj[index].size(); i++) {
        ProccessPass(adj[index][i], adj, visited, onStack, sorted);
    }

    sorted.push_back(index);
    onStack[index] = false;

    return true;
}