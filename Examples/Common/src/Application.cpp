//
// Created by alan on 15/09/2026.
//

#include "Application.h"
#include "ShaderCompiler.h"

#ifdef WIN32

#else
#define GLFW_EXPOSE_NATIVE_X11
#include <GLFW/glfw3.h>
#include <GLFW/glfw3native.h>
#include <X11/Xlib-xcb.h>
#endif

Application::Application() {
    ShaderCompiler::Init();
    glfwInit();

    CreateWindow();

    m_Device = Device::Create({ .WindowProtocol = DeviceDesc::DISPLAY_SERVER_PROTOCOL_XCB });
    m_Queue = m_Device->GetCommandQueue();

    WindowInfo windowInfo{};
    windowInfo.Xcb.Window = glfwGetX11Window(m_Window);;
    windowInfo.Xcb.Connection = XGetXCBConnection(glfwGetX11Display());

    m_Swapchain = m_Device->CreateSwapchain(windowInfo);

    CreateCubeMesh();
}

Application::~Application() {
    delete m_Cube.Index;
    delete m_Cube.Vertex;

    delete m_Swapchain;
    delete m_Device;

    glfwDestroyWindow(m_Window);
    glfwTerminate();

    ShaderCompiler::Shutdown();
}

bool Application::WindowShouldClose() {
    return glfwWindowShouldClose(m_Window);
}

void Application::BeginFrame() {
    glfwPollEvents();

    if (glfwGetKey(m_Window, GLFW_KEY_W)) {
        m_Camera.ProcessKeyboard(CameraMovement::Forward, 0.001f);
    }
    if (glfwGetKey(m_Window, GLFW_KEY_S)) {
        m_Camera.ProcessKeyboard(CameraMovement::Backward, 0.001f);
    }
    if (glfwGetKey(m_Window, GLFW_KEY_A)) {
        m_Camera.ProcessKeyboard(CameraMovement::Left, 0.001f);
    }
    if (glfwGetKey(m_Window, GLFW_KEY_D)) {
        m_Camera.ProcessKeyboard(CameraMovement::Right, 0.001f);
    }
}

void Application::EndFrame() {
    m_Device->EndFrame();
    m_Swapchain->Present();
}

void Application::CreateWindow() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_Window = glfwCreateWindow(800, 600, "RHI", nullptr, nullptr);
}

void Application::CreateCubeMesh() {
    float vertices[] = {
        -1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,

        -1.0f,-1.0f,-1.0f,
         1.0f, 1.0f,-1.0f,
         1.0f,-1.0f,-1.0f,
        -1.0f,-1.0f,-1.0f,
        -1.0f, 1.0f,-1.0f,
         1.0f, 1.0f,-1.0f,

        -1.0f,-1.0f,-1.0f,
         1.0f,-1.0f,-1.0f,
         1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f,-1.0f,
         1.0f,-1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,

        -1.0f, 1.0f,-1.0f,
        -1.0f, 1.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f,-1.0f,
         1.0f, 1.0f, 1.0f,
         1.0f, 1.0f,-1.0f,

         1.0f, 1.0f,-1.0f,
         1.0f, 1.0f, 1.0f,
         1.0f,-1.0f, 1.0f,
         1.0f,-1.0f, 1.0f,
         1.0f,-1.0f,-1.0f,
         1.0f, 1.0f,-1.0f,

        -1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
        -1.0f,-1.0f, 1.0f,
         1.0f,-1.0f, 1.0f,
         1.0f, 1.0f, 1.0f,
    };

    BufferDesc vertexDesc = {
        .Size = sizeof(vertices),
        .BindFlags = BUFFER_BIND_VERTEX | BUFFER_BIND_TRANSFER_DST,
        .Stride = 12,
        .Usage = BufferUsage::Default
    };

    uint32_t indices[] = {
        0, 1, 2, 3, 4, 5,
        6, 7, 8, 9, 10, 11,
        12, 13, 14, 15, 16, 17,
        18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29,
        28, 31, 32, 33, 34, 35,
    };

    BufferDesc indexDesc = {
        .Size = sizeof(indices),
        .BindFlags = BUFFER_BIND_INDEX | BUFFER_BIND_TRANSFER_DST,
        .Usage = BufferUsage::Default
    };

    m_Cube.Vertex = m_Device->CreateBuffer(vertexDesc);
    m_Cube.Index = m_Device->CreateBuffer(indexDesc);
    m_Cube.NumIndices = sizeof(indices) / sizeof(uint32_t);

    m_Queue->CopyToBuffer(m_Cube.Vertex, sizeof(vertices), vertices);
    m_Queue->CopyToBuffer(m_Cube.Index, sizeof(indices), indices);
    m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_VERTEX_INPUT,
    { { m_Cube.Vertex, ACCESS_TRANSFER_WRITE, ACCESS_VERTEX_READ },
                { m_Cube.Index, ACCESS_TRANSFER_WRITE, ACCESS_INDEX_READ } }, {});
}