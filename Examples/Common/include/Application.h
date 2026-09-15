//
// Created by alan on 15/09/2026.
//

#ifndef DERANGED_RHI_APPLICATION_H
#define DERANGED_RHI_APPLICATION_H

#include "Device.h"
#include "Camera.h"
#include <GLFW/glfw3.h>

struct Mesh {
    Buffer* Vertex;
    Buffer* Index;
    uint32_t NumIndices;
};

class Application {
public:
    Application();
    ~Application();

    bool WindowShouldClose();

    void BeginFrame();
    void EndFrame();

    Device* GetDevice() const { return m_Device; }
    CommandQueue* GetQueue() const { return m_Queue; }
    Swapchain* GetSwapchain() const { return m_Swapchain; }

    Camera* GetCamera() { return &m_Camera; }

    Mesh GetCubeMesh() const { return m_Cube; }

private:
    void CreateWindow();
    void CreateCubeMesh();

private:
    GLFWwindow* m_Window = nullptr;

    Device* m_Device = nullptr;
    CommandQueue* m_Queue = nullptr;
    Swapchain* m_Swapchain = nullptr;

    Camera m_Camera;

    Mesh m_Cube;

};

#endif //DERANGED_RHI_APPLICATION_H
