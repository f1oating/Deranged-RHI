//
// Created by alan on 15/09/2026.
//

#include "Application.h"
#include "ShaderCompiler.h"
#include "ThirdParty/stb_image.h"

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

    CreateSkybox();
}

Application::~Application() {
    delete m_SkyboxView;
    delete m_Skybox;

    delete m_Renderer;

    delete m_Swapchain;
    delete m_Device;

    glfwDestroyWindow(m_Window);
    glfwTerminate();

    ShaderCompiler::Shutdown();
}

void Application::Run() {
    Model* model = new Model("resources/models/cat.obj", m_Device, m_Queue);
    model->SetScale({0.05f, 0.05f, 0.05f});

    while (!glfwWindowShouldClose(m_Window)) {
        glfwPollEvents();

        CheckWindowResized();

        double newTime = glfwGetTime();
        m_DeltaTime = newTime - m_Time;
        m_Time = newTime;

        ProceedCameraMovement();

        m_Renderer->AddModel(model);
        m_Renderer->SetSkybox(m_SkyboxView);
        m_Renderer->Render();

        m_Device->EndFrame();
    }

    delete model;
}

void Application::CreateGLFWWindow() {
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    m_Window = glfwCreateWindow(800, 600, "RHI", nullptr, nullptr);
}

void Application::CreateSkybox() {
    const char* sides[] = {
        "resources/polluted_earth/polluted_earth_ft.jpg",
        "resources/polluted_earth/polluted_earth_bk.jpg",
        "resources/polluted_earth/polluted_earth_up.jpg",
        "resources/polluted_earth/polluted_earth_dn.jpg",
        "resources/polluted_earth/polluted_earth_rt.jpg",
        "resources/polluted_earth/polluted_earth_lf.jpg",
    };

    TextureDesc textureDesc = {
        .Width = 2048,
        .Height = 2048,
        .ArrayLayers = 6,
        .Dimension = TextureDimension::TextureCube,
        .BindFlags = TEXTURE_BIND_TRANSFER_DST | TEXTURE_BIND_SHADER_RESOURCE
    };
    m_Skybox = m_Device->CreateTexture(textureDesc);

    m_Queue->Barrier(PIPELINE_STAGE_NONE, PIPELINE_STAGE_TRANSFER,
    {}, { { m_Skybox, TextureLayout::TransferDST, ACCESS_NONE, ACCESS_TRANSFER_WRITE } });

    for (int i = 0; i < 6; i++) {
        int width, height, channels;
        unsigned char* loadedData = stbi_load(sides[i], &width, &height, &channels, STBI_rgb_alpha);

        m_Queue->CopyToTexture(m_Skybox, width * height * 4, loadedData, { (uint32_t)i, 1 });

        stbi_image_free(loadedData);
    }

    m_Queue->Barrier(PIPELINE_STAGE_TRANSFER, PIPELINE_STAGE_FRAGMENT_SHADER,
    {}, { { m_Skybox, TextureLayout::ShaderResource, ACCESS_TRANSFER_WRITE, ACCESS_SHADER_READ } });

    TextureViewDesc textureViewDesc = {
        .Tex = m_Skybox,
        .Dimension = TextureViewDimension::TextureCube,
        .BaseMipLevel = 0,
        .MipLevels = 1,
        .BaseArrayLayer = 0,
        .ArrayLayers = 6
    };
    m_SkyboxView = m_Device->CreateTextureView(textureViewDesc);
}

void Application::ProceedCameraMovement() {
    if (glfwGetKey(m_Window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetInputMode(m_Window, GLFW_CURSOR, m_ShowCursor ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
        m_ShowCursor = !m_ShowCursor;
    }

    if (glfwGetKey(m_Window, GLFW_KEY_W)) {
        m_Camera.ProcessMovement(Direction::Forward, m_DeltaTime);
    }
    if (glfwGetKey(m_Window, GLFW_KEY_S)) {
        m_Camera.ProcessMovement(Direction::Backward, m_DeltaTime);
    }
    if (glfwGetKey(m_Window, GLFW_KEY_A)) {
        m_Camera.ProcessMovement(Direction::Left, m_DeltaTime);
    }
    if (glfwGetKey(m_Window, GLFW_KEY_D)) {
        m_Camera.ProcessMovement(Direction::Right, m_DeltaTime);
    }

    static double posX = 0.0f;
    static double posY = 0.0f;
    double newPosX = 0.0f;
    double newPosY = 0.0f;
    glfwGetCursorPos(m_Window, &newPosX, &newPosY);

    if (posX != 0.0f && posY != 0.0f) {
        double xOffset = newPosX - posX;
#ifdef WIN32
        double yOffset = -(newPosY - posY);
#else
        double yOffset = newPosY - posY;
#endif

        m_Camera.ProcessRotation(xOffset * m_DeltaTime, yOffset * m_DeltaTime);
    }

    posX = newPosX;
    posY = newPosY;
}

void Application::CheckWindowResized() {
    int newWidth;
    int newHeight;
    glfwGetWindowSize(m_Window, &newWidth, &newHeight);
    if (m_WindowWidth != newWidth || m_WindowHeight != newHeight) {
        m_WindowWidth = newWidth;
        m_WindowHeight = newHeight;
        m_Camera.ProcessResize(m_WindowWidth, m_WindowHeight);
        m_Renderer->ResizeAttachments();
    }
}