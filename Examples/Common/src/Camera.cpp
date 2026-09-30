//
// Created by alan on 15/09/2026.
//

#include "Camera.h"

void Camera::ProcessMovement(Direction dir, double deltaTime) {
    float velocity = CameraSpeed * deltaTime;
    switch (dir) {
        case Direction::Forward:
            m_Position += m_Front * velocity;
            break;
        case Direction::Backward:
            m_Position -= m_Front * velocity;
            break;
        case Direction::Left:
            m_Position -= m_Right * velocity;
            break;
        case Direction::Right:
            m_Position += m_Right * velocity;
            break;
    }
    UpdateVectors();
}

void Camera::ProcessRotation(double xOffset, double yOffset) {
    xOffset *= CameraSpeed;
    yOffset *= CameraSpeed;

    m_Yaw += xOffset;
    m_Pitch += yOffset;

    if (m_Pitch > 89.0f) {
        m_Pitch = 89.0f;
    } else if (m_Pitch < -89.0f) {
        m_Pitch = -89.0f;
    }

    UpdateVectors();
}

void Camera::ProcessResize(int width, int height) {
    m_Width = width;
    m_Height = height;
}

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

glm::mat4 Camera::GetProjectionMatrix() {
    return glm::perspective(glm::radians(45.0f), (float)m_Width / (float)m_Height, 0.1f, 100.0f);
}

void Camera::UpdateVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));

    m_Front = glm::normalize(front);
    m_Right = glm::normalize(glm::cross(m_Front, glm::vec3(0.0f, 1.0f, 0.0f)));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}