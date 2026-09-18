//
// Created by alan on 15/09/2026.
//

#ifndef DERANGED_RHI_APPLICATION_H
#define DERANGED_RHI_APPLICATION_H

#include "Device.h"
#include "Camera.h"
#include <GLFW/glfw3.h>
#include "Renderer.h"

class Application {
public:
    Application();
    ~Application();

    void Run();

    Device* GetDevice() const { return m_Device; }
    CommandQueue* GetQueue() const { return m_Queue; }
    Swapchain* GetSwapchain() const { return m_Swapchain; }

    Camera* GetCamera() { return &m_Camera; }

    Mesh GetCubeMesh() const { return m_Cube; }

private:
    void CreateGLFWWindow();
    void CreateCubeMesh();

    void ProceedCameraMovement();
    void CheckWindowResized();

private:
    GLFWwindow* m_Window = nullptr;

    double m_Time = 0.0f;
    double m_DeltaTime = 0.0f;

    int m_WindowWidth = 800;
    int m_WindowHeight = 600;
    bool m_ShowCursor = true;

    Device* m_Device = nullptr;
    CommandQueue* m_Queue = nullptr;
    Swapchain* m_Swapchain = nullptr;

    Renderer* m_Renderer = nullptr;

    Camera m_Camera;

    Mesh m_Cube;

};

#endif //DERANGED_RHI_APPLICATION_H
