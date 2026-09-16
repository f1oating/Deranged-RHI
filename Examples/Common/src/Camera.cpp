//
// Created by alan on 15/09/2026.
//

#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 up, float yaw, float pitch)
    : m_Position(position), m_Yaw(yaw), m_Pitch(pitch), m_WorldUp(up),
    m_Front({0.0f, 0.0f, -1.0f}), m_MovementSpeed(2.5f), m_MouseSensitivity(0.1f),
    m_Zoom(45.0f), m_Width(800), m_Height(600) {
    UpdateCameraVectors();
}

void Camera::ProcessKeyboard(CameraMovement direction, float deltaTime) {
    float velocity = m_MovementSpeed * deltaTime;
    if (direction == CameraMovement::Forward) {
        m_Position += m_Front * velocity;
    }
    if (direction == CameraMovement::Backward) {
        m_Position -= m_Front * velocity;
    }
    if (direction == CameraMovement::Left) {
        m_Position -= m_Right * velocity;
    }
    if (direction == CameraMovement::Right) {
        m_Position += m_Right * velocity;
    }
}

void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch) {
    xoffset *= m_MouseSensitivity;
    yoffset *= m_MouseSensitivity;

    m_Yaw += xoffset;
    m_Pitch += yoffset;

    if (constrainPitch) {
        if (m_Pitch > 89.0f) {
            m_Pitch = 89.0f;
        }
        if (m_Pitch < -89.0f) {
            m_Pitch = -89.0f;
        }
    }

    UpdateCameraVectors();
}

void Camera::ProcessMouseScroll(float yoffset) {
    m_Zoom -= yoffset;
    if (m_Zoom < 1.0f) {
        m_Zoom = 1.0f;
    }
    if (m_Zoom > 45.0f) {
        m_Zoom = 45.0f;
    }
}

void Camera::ProcessResize(float width, float height) {
    m_Width = width;
    m_Height = height;
}

glm::mat4 Camera::GetViewMatrix() {
    return glm::lookAt(m_Position, m_Position + m_Front, m_Up);
}

glm::mat4 Camera::GetProjectionMatrix() {
    return glm::perspective(glm::radians(m_Zoom), m_Width / m_Height, 0.1f, 100.0f);
}

void Camera::UpdateCameraVectors() {
    glm::vec3 front;
    front.x = cos(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    front.y = sin(glm::radians(m_Pitch));
    front.z = sin(glm::radians(m_Yaw)) * cos(glm::radians(m_Pitch));
    m_Front = glm::normalize(front);

    m_Right = glm::normalize(glm::cross(m_Front, m_WorldUp));
    m_Up = glm::normalize(glm::cross(m_Right, m_Front));
}