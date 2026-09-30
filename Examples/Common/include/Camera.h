//
// Created by alan on 15/09/2026.
//

#ifndef DERANGED_RHI_CAMERA_H
#define DERANGED_RHI_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class Direction {
    Forward,
    Backward,
    Left,
    Right
};

constexpr float CameraSpeed = 5.0f;

class Camera {
public:
    void ProcessMovement(Direction dir, double deltaTime);
    void ProcessRotation(double xOffset, double yOffset);

    void ProcessResize(int width, int height);

    glm::mat4 GetViewMatrix();
    glm::mat4 GetProjectionMatrix();

private:
    void UpdateVectors();

private:
    glm::vec3 m_Position = { 0.0f, 0.0f, 0.0f };
    glm::vec3 m_Front = { 0.0f, 0.0f, 1.0f };
    glm::vec3 m_Up = { 0.0f, 1.0f, 0.0f };
    glm::vec3 m_Right = { 1.0f, 0.0f, 0.0f };

    float m_Yaw = 0.0f;
    float m_Pitch = 0.0f;

    int m_Width = 800;
    int m_Height = 600;

};

#endif //DERANGED_RHI_CAMERA_H
