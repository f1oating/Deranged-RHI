//
// Created by alan on 15/09/2026.
//

#ifndef DERANGED_RHI_CAMERA_H
#define DERANGED_RHI_CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovement {
    Forward,
    Backward,
    Left,
    Right
};

class Camera {
public:
    Camera(glm::vec3 position = { 0.0f, 0.0f, 0.0f }, glm::vec3 up = { 0.0f, 1.0f, 0.0f },
        float yaw = -90.0f, float pitch = 0.0f);

    void ProcessKeyboard(CameraMovement direction, float deltaTime);
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);
    void ProcessMouseScroll(float yoffset);
    void ProcessResize(float width, float height);

    glm::mat4 GetViewMatrix();
    glm::mat4 GetProjectionMatrix();

private:
    void UpdateCameraVectors();

private:
    glm::vec3 m_Position;
    glm::vec3 m_Front;
    glm::vec3 m_Up;
    glm::vec3 m_Right;
    glm::vec3 m_WorldUp;
    float m_Yaw;
    float m_Pitch;
    float m_MovementSpeed;
    float m_MouseSensitivity;
    float m_Zoom;
    float m_Width;
    float m_Height;

};

#endif //DERANGED_RHI_CAMERA_H
