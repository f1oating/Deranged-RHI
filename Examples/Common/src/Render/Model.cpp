//
// Created by alan on 13/11/2026.
//

#include "Render/Model.h"
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

Model::Model(const char* path, Device* device, CommandQueue* queue) {
    m_Device = device;
    m_Queue = queue;

    LoadModel(path);
}

Model::~Model() {
    if (m_Mesh.Vertex) delete m_Mesh.Vertex;
    if (m_Mesh.Index) delete m_Mesh.Index;
    if (m_Mesh.Albedo) delete m_Mesh.Albedo;
}

glm::mat4 Model::GetWorldMatrix() {
    glm::mat4 worldMatrix = glm::mat4(1.0f);
    worldMatrix = glm::translate(worldMatrix, m_Position);
    worldMatrix = glm::scale(worldMatrix, m_Scale);
    return worldMatrix;
}

void Model::LoadModel(const char* path) {
    Assimp::Importer importer;

    const aiScene* scene = importer.ReadFile(path, 0);

    aiMesh* mesh = scene->mMeshes[0];
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    for (uint32_t i = 0; i < mesh->mNumVertices; ++i) {
        Vertex vertex;
        vertex.Position = { mesh->mVertices[i].x, mesh->mVertices[i].y, mesh->mVertices[i].z };

        vertices.push_back(vertex);
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; ++i) {
        aiFace face = mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; ++j) {
            indices.push_back(face.mIndices[j]);
        }
    }

    BufferDesc vertexDesc = {
        .Size = vertices.size() * sizeof(Vertex),
        .BindFlags = BUFFER_BIND_VERTEX | BUFFER_BIND_TRANSFER_DST,
        .Stride = 32,
        .Usage = BufferUsage::Default
    };

    BufferDesc indexDesc = {
        .Size = indices.size() * sizeof(uint32_t),
        .BindFlags = BUFFER_BIND_INDEX | BUFFER_BIND_TRANSFER_DST,
        .Usage = BufferUsage::Default
    };

    m_Mesh.Vertex = m_Device->CreateBuffer(vertexDesc);
    m_Mesh.Index = m_Device->CreateBuffer(indexDesc);
    m_Mesh.NumIndices = indices.size();

    m_Queue->CopyToBuffer(m_Mesh.Vertex, vertices.size() * sizeof(Vertex), vertices.data());
    m_Queue->CopyToBuffer(m_Mesh.Index, indices.size() * sizeof(uint32_t), indices.data());

    m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_VERTEX_INPUT,
    { { m_Mesh.Vertex, ACCESS_TRANSFER_WRITE, ACCESS_VERTEX_READ } }, {});
    m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_INDEX_INPUT,
    { { m_Mesh.Index, ACCESS_TRANSFER_WRITE, ACCESS_INDEX_READ } }, {});
}
