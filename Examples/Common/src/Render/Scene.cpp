//
// Created by alan on 13/11/2026.
//

#include "Render/Scene.h"
#include <glm/gtc/matrix_transform.hpp>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "ThirdParty/stb_image.h"
#include <string>

Model::Model(const char* path, Device* device, CommandQueue* queue) {
    m_Device = device;
    m_Queue = queue;

    LoadModel(path);
}

Model::~Model() {
    for (auto material : m_Materials) {
        if (material.Diffuse) delete material.Diffuse;
        if (material.Diffuse) delete material.Normal;
    }
    m_Materials.clear();
    for (auto mesh : m_Meshes) {
        if (mesh.Vertex) delete mesh.Vertex;
        if (mesh.Index) delete mesh.Index;
    }
    m_Meshes.clear();
    if (m_DebugMesh.Vertex) delete m_DebugMesh.Vertex;
    if (m_DebugMesh.Index) delete m_DebugMesh.Index;
}

glm::mat4 Model::GetWorldMatrix() {
    glm::mat4 worldMatrix = glm::mat4(1.0f);
    worldMatrix = glm::translate(worldMatrix, m_Position);
    worldMatrix = glm::scale(worldMatrix, m_Scale);
    return worldMatrix;
}

void Model::LoadModel(const char* path) {
    Assimp::Importer importer;

    const aiScene* aiScene = importer.ReadFile(path, aiProcess_FlipUVs | aiProcess_CalcTangentSpace |
        aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_MakeLeftHanded);

    for (int materialIndex = 0; materialIndex < 1; ++materialIndex) {
        aiMaterial* aiMaterial = aiScene->mMaterials[materialIndex];
        Material material{};

        aiString path;
        aiMaterial->Get(AI_MATKEY_TEXTURE_DIFFUSE(0), path);

        if (true) {
            int width, height, channels;
            std::string fullPath = std::string("resources/models/sharp-boulder2-albedo.png");
            std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
            unsigned char* loadedData = stbi_load(fullPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

            TextureDesc textureDesc = {
                .Width = (uint32_t)width,
                .Height = (uint32_t)height,
                .ArrayLayers = 1,
                .BindFlags = TEXTURE_BIND_TRANSFER_DST | TEXTURE_BIND_SHADER_RESOURCE
            };
            material.Diffuse = m_Device->CreateTexture(textureDesc);

            m_Queue->Barrier(PIPELINE_STAGE_NONE, PIPELINE_STAGE_TRANSFER,
            {}, { { material.Diffuse, TextureLayout::TransferDST, ACCESS_NONE, ACCESS_TRANSFER_WRITE } });

            m_Queue->CopyToTexture(material.Diffuse, width * height * 4, loadedData, { 0, 1 });

            stbi_image_free(loadedData);

            m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_FRAGMENT_SHADER,
            {}, { { material.Diffuse, TextureLayout::ShaderResource, ACCESS_TRANSFER_WRITE, ACCESS_SHADER_READ } });
        }

        aiMaterial->Get(AI_MATKEY_TEXTURE_NORMALS(0), path);

        if (true) {
            int width, height, channels;
            std::string fullPath = std::string("resources/models/sharp-boulder2-normal_ogl.png");
            std::replace(fullPath.begin(), fullPath.end(), '\\', '/');
            unsigned char* loadedData = stbi_load(fullPath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

            TextureDesc textureDesc = {
                .Width = (uint32_t)width,
                .Height = (uint32_t)height,
                .ArrayLayers = 1,
                .BindFlags = TEXTURE_BIND_TRANSFER_DST | TEXTURE_BIND_SHADER_RESOURCE
            };
            material.Normal = m_Device->CreateTexture(textureDesc);

            m_Queue->Barrier(PIPELINE_STAGE_NONE, PIPELINE_STAGE_TRANSFER,
            {}, { { material.Normal, TextureLayout::TransferDST, ACCESS_NONE, ACCESS_TRANSFER_WRITE } });

            m_Queue->CopyToTexture(material.Normal, width * height * 4, loadedData, { 0, 1 });

            stbi_image_free(loadedData);

            m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_FRAGMENT_SHADER,
            {}, { { material.Normal, TextureLayout::ShaderResource, ACCESS_TRANSFER_WRITE, ACCESS_SHADER_READ } });
        }

        m_Materials.push_back(material);
    }

    for (int meshIndex = 0; meshIndex < aiScene->mNumMeshes; meshIndex++) {
        aiMesh* aiMesh = aiScene->mMeshes[meshIndex];

        std::vector<VertexPositionTexture> vertices;
        std::vector<uint32_t> indices;

        for (uint32_t i = 0; i < aiMesh->mNumVertices; ++i) {
            VertexPositionTexture vertex{};

            vertex.Position = { aiMesh->mVertices[i].x, aiMesh->mVertices[i].y, aiMesh->mVertices[i].z };
            vertex.UV = { aiMesh->mTextureCoords[0][i].x, aiMesh->mTextureCoords[0][i].y };
            vertex.Normal = { aiMesh->mNormals[i].x, aiMesh->mNormals[i].y, aiMesh->mNormals[i].z };
            vertex.Tangent = { aiMesh->mTangents[i].x, aiMesh->mTangents[i].y, aiMesh->mTangents[i].z };
            vertex.Bitangent = { aiMesh->mBitangents[i].x, aiMesh->mBitangents[i].y, aiMesh->mBitangents[i].z };

            vertices.push_back(vertex);
        }

        for (uint32_t i = 0; i < aiMesh->mNumFaces; ++i) {
            aiFace face = aiMesh->mFaces[i];
            for (uint32_t j = 0; j < face.mNumIndices; ++j) {
                indices.push_back(face.mIndices[j]);
            }
        }

        std::vector<VertexPositionColor> debugVertices;
        std::vector<uint32_t> debugIndices;
        uint32_t vertexIndex = 0;
        for (uint32_t i = 0; i < indices.size(); ++i) {
            uint32_t index = indices[i];
            VertexPositionTexture vertex = vertices[index];

            VertexPositionColor vert1n = { vertex.Position, { 1.0f, 1.0f, 1.0f } };
            VertexPositionColor vert2n = { vertex.Position + glm::normalize(vertex.Normal) * 0.1f, { 1.0f, 1.0f, 1.0f } };

            debugVertices.push_back(vert1n);
            debugVertices.push_back(vert2n);
            debugIndices.push_back(vertexIndex++);
            debugIndices.push_back(vertexIndex++);

            VertexPositionColor vert1t = { vertex.Position, { 1.0f, 0.0f, 0.0f } };
            VertexPositionColor vert2t = { vertex.Position + glm::normalize(vertex.Tangent) * 0.1f, { 1.0f, 0.0f, 0.0f } };

            debugVertices.push_back(vert1t);
            debugVertices.push_back(vert2t);
            debugIndices.push_back(vertexIndex++);
            debugIndices.push_back(vertexIndex++);

            VertexPositionColor vert1b = { vertex.Position, { 0.0f, 0.0f, 1.0f } };
            VertexPositionColor vert2b = { vertex.Position + glm::normalize(vertex.Bitangent) * 0.1f, { 0.0f, 0.0f, 1.0f } };;

            debugVertices.push_back(vert1b);
            debugVertices.push_back(vert2b);
            debugIndices.push_back(vertexIndex++);
            debugIndices.push_back(vertexIndex++);
        }

        BufferDesc debugVertexDesc = {
            .Size = debugVertices.size() * sizeof(VertexPositionColor),
            .BindFlags = BUFFER_BIND_VERTEX | BUFFER_BIND_TRANSFER_DST,
            .Stride = sizeof(VertexPositionColor),
        };

        BufferDesc debugIndexDesc = {
            .Size = debugIndices.size() * sizeof(uint32_t),
            .BindFlags = BUFFER_BIND_INDEX | BUFFER_BIND_TRANSFER_DST,
        };

        BufferDesc vertexDesc = {
            .Size = vertices.size() * sizeof(VertexPositionTexture),
            .BindFlags = BUFFER_BIND_VERTEX | BUFFER_BIND_TRANSFER_DST,
            .Stride = sizeof(VertexPositionTexture),
        };

        BufferDesc indexDesc = {
            .Size = indices.size() * sizeof(uint32_t),
            .BindFlags = BUFFER_BIND_INDEX | BUFFER_BIND_TRANSFER_DST,
        };

        Mesh mesh{};
        mesh.Vertex = m_Device->CreateBuffer(vertexDesc);
        mesh.Index = m_Device->CreateBuffer(indexDesc);
        mesh.MaterialIndex = 0;
        mesh.NumIndices = indices.size();

        m_DebugMesh.Vertex = m_Device->CreateBuffer(debugVertexDesc);
        m_DebugMesh.Index = m_Device->CreateBuffer(debugIndexDesc);
        m_DebugMesh.NumIndices = debugIndices.size();

        m_Queue->CopyToBuffer(mesh.Vertex, vertices.size() * sizeof(VertexPositionTexture), vertices.data());
        m_Queue->CopyToBuffer(mesh.Index, indices.size() * sizeof(uint32_t), indices.data());

        m_Queue->CopyToBuffer(m_DebugMesh.Vertex, debugVertices.size() * sizeof(VertexPositionColor), debugVertices.data());
        m_Queue->CopyToBuffer(m_DebugMesh.Index, debugIndices.size() * sizeof(uint32_t), debugIndices.data());

        m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_VERTEX_INPUT,
        { { mesh.Vertex, ACCESS_TRANSFER_WRITE, ACCESS_VERTEX_READ } }, {});
        m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_INDEX_INPUT,
        { { mesh.Index, ACCESS_TRANSFER_WRITE, ACCESS_INDEX_READ } }, {});

        m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_VERTEX_INPUT,
        { { m_DebugMesh.Vertex, ACCESS_TRANSFER_WRITE, ACCESS_VERTEX_READ } }, {});
        m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_INDEX_INPUT,
        { { m_DebugMesh.Index, ACCESS_TRANSFER_WRITE, ACCESS_INDEX_READ } }, {});

        m_Meshes.push_back(mesh);
    }
}
