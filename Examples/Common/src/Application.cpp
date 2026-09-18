//
// Created by alan on 15/09/2026.
//

#include "Application.h"
#include "ShaderCompiler.h"

#ifdef WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#else
#define GLFW_EXPOSE_NATIVE_X11
#include <X11/Xlib-xcb.h>
#include <GLFW/glfw3native.h>
#endif

Application::Application() {
    ShaderCompiler::Init();
    glfwInit();

    m_Time = glfwGetTime();

    CreateGLFWWindow();

    DeviceDesc deviceDesc;
    WindowInfo windowInfo{};
#ifdef WIN32
    windowInfo.Window = glfwGetWin32Window(m_Window);
    windowInfo.Instance = GetModuleHandle(nullptr);
#else
    deviceDesc.WindowProtocol = DeviceDesc::DISPLAY_SERVER_PROTOCOL_XCB;
    windowInfo.Xcb.Window = glfwGetX11Window(m_Window);
    windowInfo.Xcb.Connection = XGetXCBConnection(glfwGetX11Display());
#endif

    m_Device = Device::Create(deviceDesc);
    m_Queue = m_Device->GetCommandQueue();

    m_Swapchain = m_Device->CreateSwapchain(windowInfo);

    m_Renderer = new Renderer(m_Device, m_Queue, m_Swapchain);
    m_Renderer->SetCamera(&m_Camera);

    CreateCubeMesh();
}

Application::~Application() {
    delete m_Cube.Index;
    delete m_Cube.Vertex;

    delete m_Renderer;

    delete m_Swapchain;
    delete m_Device;

    glfwDestroyWindow(m_Window);
    glfwTerminate();

    ShaderCompiler::Shutdown();
}

void Application::Run() {
    while (!glfwWindowShouldClose(m_Window)) {
        glfwPollEvents();

        CheckWindowResized();

        double newTime = glfwGetTime();
        m_DeltaTime = newTime - m_Time;
        m_Time = newTime;

        ProceedCameraMovement();

        m_Renderer->BeginFrame();
        m_Renderer->Render(m_Cube);
        m_Renderer->EndFrame();

        m_Device->EndFrame();
    }
}

void Application::CreateGLFWWindow() {
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
    { { m_Cube.Vertex, ACCESS_TRANSFER_WRITE, ACCESS_VERTEX_READ } }, {});
    m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_INDEX_INPUT,
    { { m_Cube.Index, ACCESS_TRANSFER_WRITE, ACCESS_INDEX_READ } }, {});
}

void Application::ProceedCameraMovement() {
    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetInputMode(m_Window, GLFW_CURSOR, m_ShowCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        m_ShowCursor = !m_ShowCursor;
    }

    if (glfwGetKey(m_Window, GLFW_KEY_W)) {
        m_Camera.ProcessKeyboard(CameraMovement::Forward, m_DeltaTime);
    }
    if (glfwGetKey(m_Window, GLFW_KEY_S)) {
        m_Camera.ProcessKeyboard(CameraMovement::Backward, m_DeltaTime);
    }
    if (glfwGetKey(m_Window, GLFW_KEY_A)) {
        m_Camera.ProcessKeyboard(CameraMovement::Left, m_DeltaTime);
    }
    if (glfwGetKey(m_Window, GLFW_KEY_D)) {
        m_Camera.ProcessKeyboard(CameraMovement::Right, m_DeltaTime);
    }

    if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_LEFT)) {
        m_Camera.ProcessMouseScroll(10.0f * m_DeltaTime);
    }
    if (glfwGetMouseButton(m_Window, GLFW_MOUSE_BUTTON_RIGHT)) {
        m_Camera.ProcessMouseScroll(-10.0f * m_DeltaTime);
    }

    static double posX = 0.0f;
    static double posY = 0.0f;
    double newPosX = 0.0f;
    double newPosY = 0.0f;
    glfwGetCursorPos(m_Window, &newPosX, &newPosY);

    m_Camera.ProcessMouseMovement(newPosX - posX, newPosY - posY, true);
    posX = newPosX;
    posY = newPosY;

    int width = 1;
    int height = 1;
    glfwGetWindowSize(m_Window, &width, &height);
    m_Camera.ProcessResize(width, height);
}

void Application::CheckWindowResized() {
    int newWidth;
    int newHeight;
    glfwGetWindowSize(m_Window, &newWidth, &newHeight);
    if (m_WindowWidth != newWidth || m_WindowHeight != newHeight) {
        m_WindowWidth = newWidth;
        m_WindowHeight = newHeight;
        m_Renderer->ResizeAttachments();
    }
}