//
// Created by alan on 13/11/2026.
//

#ifndef DERANGED_RHI_SCENE_H
#define DERANGED_RHI_SCENE_H

#include <glm/glm.hpp>
#include <Device.h>
#include <vector>

struct VertexPositionTexture {
    glm::vec3 Position;
    glm::vec2 UV;
    glm::vec3 Normal;
    glm::vec3 Tangent;
    glm::vec3 Bitangent;
};

struct VertexPositionTextureNormal {
    glm::vec3 Position;
    glm::vec2 UV;
    glm::vec3 Normal;
};

struct Material {
    Texture* Diffuse;
    Texture* Normal;
};

struct Mesh {
    Buffer* Vertex;
    Buffer* Index;
    uint32_t MaterialIndex;
    uint32_t NumIndices;
};

class Model {
public:
    Model(const char* path, Device* device, CommandQueue* queue);
    ~Model();

    void SetPosition(glm::vec3 pos) { m_Position = pos; }
    void SetScale(glm::vec3 scale) { m_Scale = scale; }

    Material GetMaterial(uint32_t index) { return m_Materials[index]; }

    uint32_t GetNumMeshes() { return m_Meshes.size(); }
    Mesh GetMesh(uint32_t index) { return m_Meshes[index]; }

    glm::mat4 GetWorldMatrix();

private:
    void LoadModel(const char* path);

private:
    Device* m_Device = nullptr;
    CommandQueue* m_Queue = nullptr;

    std::vector<Material> m_Materials{};
    std::vector<Mesh> m_Meshes{};

    glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_Scale = { 1.0f, 1.0f, 1.0f };

};

struct Transform {
    glm::mat4 ModelViewProjection;
    glm::mat4 TransInvModelView;
    glm::mat4 Model;
};

struct Light {
    glm::vec3 Directional;
};

#endif //DERANGED_RHI_SCENE_H
