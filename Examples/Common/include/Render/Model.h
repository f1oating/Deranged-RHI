//
// Created by alan on 13/11/2026.
//

#ifndef DERANGED_RHI_MODEL_H
#define DERANGED_RHI_MODEL_H

#include <glm/glm.hpp>
#include <Device.h>

struct Vertex {
    glm::vec3 Position;
    glm::vec2 UV;
    glm::vec3 Normal;
};

struct Mesh {
    Buffer* Vertex;
    Buffer* Index;
    Texture* Albedo;
    uint32_t NumIndices;
};

class Model {
public:
    Model(const char* path, Device* device, CommandQueue* queue);
    ~Model();

    void SetPosition(glm::vec3 pos) { m_Position = pos; }
    void SetScale(glm::vec3 scale) { m_Scale = scale; }

    Mesh GetMesh() { return m_Mesh; }

    glm::mat4 GetWorldMatrix();

private:
    void LoadModel(const char* path);

private:
    Device* m_Device = nullptr;
    CommandQueue* m_Queue = nullptr;

    Mesh m_Mesh{};

    glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_Scale = { 1.0f, 1.0f, 1.0f };

};

#endif //DERANGED_RHI_MODEL_H
