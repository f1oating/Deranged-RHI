//
// Created by alan on 13/11/2026.
//

#include "Render/Scene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Model::Model(const char* path, Device* device, CommandQueue* queue) {
    m_Device = device;
    m_Queue = queue;

    LoadModel(path);
}

Model::~Model() {
    for (auto mesh : m_Meshes) {
        if (mesh.Vertex) delete mesh.Vertex;
        if (mesh.Index) delete mesh.Index;
        if (mesh.Albedo) delete mesh.Albedo;
    }
    m_Meshes.clear();
}

glm::mat4 Model::GetWorldMatrix() {
    glm::mat4 worldMatrix = glm::mat4(1.0f);
    worldMatrix = glm::translate(worldMatrix, m_Position);
    worldMatrix = glm::scale(worldMatrix, m_Scale);
    return worldMatrix;
}

void Model::LoadModel(const char* path) {
    Assimp::Importer importer;

    const aiScene* aiScene = importer.ReadFile(path, aiProcess_GenSmoothNormals);

    for (int meshIndex = 0; meshIndex < aiScene->mNumMeshes; meshIndex++) {
        aiMesh* aiMesh = aiScene->mMeshes[meshIndex];

        std::vector<Vertex> vertices;
        std::vector<uint32_t> indices;

        for (uint32_t i = 0; i < aiMesh->mNumVertices; ++i) {
            Vertex vertex;
            vertex.Position = { aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z };
            vertex.Normal = { aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z };
            vertex.UV = { aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y };

            vertices.push_back(vertex);
        }

        for (uint32_t i = 0; i < aiMesh->mNumFaces; ++i) {
            aiFace face = aiMesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; ++j) {
                indices.push_back(face.mIndices[j]);
            }
        }

        aiMaterial* material = aiScene->mMaterials[aiMesh->mMaterialIndex];

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

        Mesh mesh{};
        mesh.Vertex = m_Device->CreateBuffer(vertexDesc);
        mesh.Index = m_Device->CreateBuffer(indexDesc);
        mesh.NumIndices = indices.size();

        m_Queue->CopyToBuffer(mesh.Vertex, vertices.size() * sizeof(Vertex), vertices.data());
        m_Queue->CopyToBuffer(mesh.Index, indices.size() * sizeof(uint32_t), indices.data());

        m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_VERTEX_INPUT,
        { { mesh.Vertex, ACCESS_TRANSFER_WRITE, ACCESS_VERTEX_READ } }, {});
        m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_INDEX_INPUT,
        { { mesh.Index, ACCESS_TRANSFER_WRITE, ACCESS_INDEX_READ } }, {});

        m_Meshes.push_back(mesh);
    }
}
