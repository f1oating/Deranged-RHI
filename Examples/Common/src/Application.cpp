//
// Created by alan on 15/09/2026.
//

#include "Application.h"
#include "ShaderCompiler.h"
#include "ThirdParty/stb_image.h"
#include <memory>
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
    delete m_Cube.Albedo;

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

        m_Renderer->AddMesh(m_Cube);
        m_Renderer->Render();

        m_Device->EndFrame();
    }
}

void Application::CreateGLFWWindow() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_Window = glfwCreateWindow(800, 600, "RHI", nullptr, nullptr);
}

void Application::CreateCubeMesh() {
    glm::f32 x = 1.0f / 2.0f;
    glm::f32 y = 1.0f / 2.0f;
    glm::f32 z = 1.0f / 2.0f;

    glm::vec3 a0 = glm::vec3(+x, +y, +z);
    glm::vec3 a1 = glm::vec3(-x, +y, +z);
    glm::vec3 a2 = glm::vec3(-x, -y, +z);
    glm::vec3 a3 = glm::vec3(+x, -y, +z);
    glm::vec3 a4 = glm::vec3(+x, +y, -z);
    glm::vec3 a5 = glm::vec3(-x, +y, -z);
    glm::vec3 a6 = glm::vec3(-x, -y, -z);
    glm::vec3 a7 = glm::vec3(+x, -y, -z);

    glm::vec3 verts[] = {
        a1, a2, a3, a3, a0, a1,
        a2, a6, a7, a7, a3, a2,
        a6, a5, a4, a4, a7, a6,
        a5, a1, a0, a0, a4, a5,
        a0, a3, a7, a7, a4, a0,
        a5, a6, a2, a2, a1, a5
    };

    glm::vec2 texc[] = {
        glm::vec2(0,1), glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,0), glm::vec2(1,1), glm::vec2(0,1),
        glm::vec2(0,1), glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,0), glm::vec2(1,1), glm::vec2(0,1),
        glm::vec2(1,0), glm::vec2(1,1), glm::vec2(0,1), glm::vec2(0,1), glm::vec2(0,0), glm::vec2(1,0),
        glm::vec2(0,1), glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,0), glm::vec2(1,1), glm::vec2(0,1),
        glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,1), glm::vec2(1,1), glm::vec2(0,1), glm::vec2(0,0),
        glm::vec2(1,1), glm::vec2(0,1), glm::vec2(0,0), glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,1),
};

    glm::vec3 norm[36];

    for (int i = 0; i < 36; i += 3)
    {
        glm::vec3 normal = glm::normalize(
            glm::cross(
                glm::vec3(verts[i + 1]) - glm::vec3(verts[i]),
                glm::vec3(verts[i + 2]) - glm::vec3(verts[i])));

        norm[i] = normal;
        norm[i + 1] = normal;
        norm[i + 2] = normal;
    }

    float vertices[36 * 8];
    for (int i = 0; i < 36; i++)
    {
        vertices[i * 8 + 0] = verts[i].x;
        vertices[i * 8 + 1] = verts[i].y;
        vertices[i * 8 + 2] = verts[i].z;

        vertices[i * 8 + 3] = texc[i].x;
        vertices[i * 8 + 4] = texc[i].y;

        vertices[i * 8 + 5] = norm[i].x;
        vertices[i * 8 + 6] = norm[i].y;
        vertices[i * 8 + 7] = norm[i].z;
    }


    BufferDesc vertexDesc = {
        .Size = sizeof(vertices),
        .BindFlags = BUFFER_BIND_VERTEX | BUFFER_BIND_TRANSFER_DST,
        .Stride = 32,
        .Usage = BufferUsage::Default
    };

    uint32_t indices[] = {
        0, 1, 2, 3, 4, 5,
        6, 7, 8, 9, 10, 11,
        12, 13, 14, 15, 16, 17,
        18, 19, 20, 21, 22, 23,
        24, 25, 26, 27, 28, 29,
        30, 31, 32, 33, 34, 35,
    };

    BufferDesc indexDesc = {
        .Size = sizeof(indices),
        .BindFlags = BUFFER_BIND_INDEX | BUFFER_BIND_TRANSFER_DST,
        .Usage = BufferUsage::Default
    };

    int width, height, channels;
    unsigned char* loadedData = stbi_load("resources/metal.jpg", &width, &height, &channels, STBI_rgb_alpha);

    TextureDesc textureDesc = {
        .Width = (uint32_t)width,
        .Height = (uint32_t)height,
        .BindFlags = TEXTURE_BIND_TRANSFER_DST | TEXTURE_BIND_SHADER_RESOURCE
    };

    m_Cube.Vertex = m_Device->CreateBuffer(vertexDesc);
    m_Cube.Index = m_Device->CreateBuffer(indexDesc);
    m_Cube.Albedo = m_Device->CreateTexture(textureDesc);
    m_Cube.NumIndices = sizeof(indices) / sizeof(uint32_t);

    m_Queue->Barrier(PIPELINE_STAGE_NONE, PIPELINE_STAGE_TRANSFER,
    {}, { { m_Cube.Albedo, ImageLayout::TransferDST, ACCESS_NONE, ACCESS_TRANSFER_WRITE } });

    m_Queue->CopyToBuffer(m_Cube.Vertex, sizeof(vertices), vertices);
    m_Queue->CopyToBuffer(m_Cube.Index, sizeof(indices), indices);
    m_Queue->CopyToTexture(m_Cube.Albedo, width * height * 4, loadedData, {});

    stbi_image_free(loadedData);

    m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_VERTEX_INPUT,
    { { m_Cube.Vertex, ACCESS_TRANSFER_WRITE, ACCESS_VERTEX_READ } }, {});
    m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_INDEX_INPUT,
    { { m_Cube.Index, ACCESS_TRANSFER_WRITE, ACCESS_INDEX_READ } }, {});
    m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_FRAGMENT_SHADER,
    {}, { { m_Cube.Albedo, ImageLayout::ShaderResource, ACCESS_TRANSFER_WRITE, ACCESS_SHADER_READ } });
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