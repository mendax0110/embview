#include "ui/Application.h"

#include "core/DataStore.h"
#include "core/DeviceManager.h"
#include "core/LogFileManager.h"
#include "ui/MainWindow.h"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <implot.h>
#include <GLFW/glfw3.h>
#include <spdlog/spdlog.h>

using namespace embview::ui;

Application::Application() = default;

Application::~Application()
{
    shutdown();
}

bool Application::init(std::shared_ptr<core::LogFileManager> logFileMgr, const std::string& title, const int width, const int height)
{
    if (!glfwInit())
    {
        spdlog::error("Failed to initialize GLFW");
        return false;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    m_window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
    if (!m_window)
    {
        spdlog::error("Failed to create GLFW window");
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1); // vsync

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    ImGui_ImplGlfw_InitForOpenGL(m_window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Query DPI scale from the primary monitor
    float xscale = 1.0f;
    float yscale = 1.0f;
    glfwGetWindowContentScale(m_window, &xscale, &yscale);
    applyDpiScale(xscale);

    m_dataStore = std::make_shared<core::DataStore>();
    m_deviceMgr = std::make_shared<core::DeviceManager>(m_dataStore);

    m_mainWindow = std::make_unique<MainWindow>(m_dataStore, m_deviceMgr, std::move(logFileMgr));

    m_initialized = true;
    spdlog::info("Application initialized");
    return true;
}

void Application::run()
{
    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();

        try
        {
            // Detect DPI scale changes (e.g. window dragged to different monitor)
            float xscale = 1.0f;
            float yscale = 1.0f;
            glfwGetWindowContentScale(m_window, &xscale, &yscale);
            if (xscale != m_currentDpiScale)
            {
                applyDpiScale(xscale);
                m_mainWindow->requestLayoutRebuild();
            }

            // DeviceManager reader threads handle all transport I/O

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            m_mainWindow->render();

            if (m_mainWindow->shouldClose())
            {
                glfwSetWindowShouldClose(m_window, GLFW_TRUE);
            }

            ImGui::Render();

            int displayW, displayH;
            glfwGetFramebufferSize(m_window, &displayW, &displayH);
            glViewport(0, 0, displayW, displayH);
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        }
        catch (const std::exception& e)
        {
            spdlog::error("Frame error: {}", e.what());
        }

        glfwSwapBuffers(m_window);
    }
}

void Application::shutdown()
{
    if (!m_initialized)
    {
        return;
    }
    m_initialized = false;

    m_mainWindow.reset();

    if (m_deviceMgr)
    {
        m_deviceMgr->removeAll();
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    if (m_window)
    {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }

    glfwTerminate();
    spdlog::info("Application shutdown");
}

void Application::applyDpiScale(float scale)
{
    if (scale < 0.5f)
    {
        scale = 1.0f;
    }
    m_currentDpiScale = scale;

    const ImGuiIO& io = ImGui::GetIO();
    io.Fonts->Clear();

    ImFontConfig fontCfg;
    fontCfg.SizePixels = 13.0f * scale;
    fontCfg.OversampleH = 2;
    fontCfg.OversampleV = 1;
    io.Fonts->AddFontDefault(&fontCfg);
    io.Fonts->Build();

    ImGui_ImplOpenGL3_DestroyFontsTexture();

    // Reset to ImGui defaults, then apply only colors and scale
    ImGui::StyleColorsDark();
    ImGuiStyle& style = ImGui::GetStyle();

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg]             = ImVec4(0.11f, 0.11f, 0.13f, 1.00f);
    colors[ImGuiCol_ChildBg]              = ImVec4(0.11f, 0.11f, 0.13f, 1.00f);
    colors[ImGuiCol_PopupBg]              = ImVec4(0.13f, 0.13f, 0.16f, 0.96f);
    colors[ImGuiCol_Border]               = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
    colors[ImGuiCol_FrameBg]              = ImVec4(0.16f, 0.16f, 0.19f, 1.00f);
    colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
    colors[ImGuiCol_FrameBgActive]        = ImVec4(0.28f, 0.28f, 0.33f, 1.00f);
    colors[ImGuiCol_TitleBg]              = ImVec4(0.09f, 0.09f, 0.11f, 1.00f);
    colors[ImGuiCol_TitleBgActive]        = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.09f, 0.09f, 0.11f, 0.75f);
    colors[ImGuiCol_MenuBarBg]            = ImVec4(0.12f, 0.12f, 0.15f, 1.00f);
    colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.11f, 0.11f, 0.13f, 1.00f);
    colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.25f, 0.25f, 0.30f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.30f, 0.30f, 0.36f, 1.00f);
    colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.35f, 0.35f, 0.42f, 1.00f);
    colors[ImGuiCol_CheckMark]            = ImVec4(0.30f, 0.65f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrab]           = ImVec4(0.30f, 0.65f, 0.90f, 1.00f);
    colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.40f, 0.72f, 0.95f, 1.00f);
    colors[ImGuiCol_Button]               = ImVec4(0.20f, 0.20f, 0.24f, 1.00f);
    colors[ImGuiCol_ButtonHovered]        = ImVec4(0.28f, 0.28f, 0.33f, 1.00f);
    colors[ImGuiCol_ButtonActive]         = ImVec4(0.30f, 0.65f, 0.90f, 0.80f);
    colors[ImGuiCol_Header]               = ImVec4(0.20f, 0.20f, 0.24f, 1.00f);
    colors[ImGuiCol_HeaderHovered]        = ImVec4(0.28f, 0.28f, 0.33f, 1.00f);
    colors[ImGuiCol_HeaderActive]         = ImVec4(0.30f, 0.65f, 0.90f, 0.80f);
    colors[ImGuiCol_Separator]            = ImVec4(0.22f, 0.22f, 0.26f, 1.00f);
    colors[ImGuiCol_SeparatorHovered]     = ImVec4(0.30f, 0.65f, 0.90f, 0.60f);
    colors[ImGuiCol_SeparatorActive]      = ImVec4(0.30f, 0.65f, 0.90f, 1.00f);
    colors[ImGuiCol_ResizeGrip]           = ImVec4(0.30f, 0.65f, 0.90f, 0.20f);
    colors[ImGuiCol_ResizeGripHovered]    = ImVec4(0.30f, 0.65f, 0.90f, 0.60f);
    colors[ImGuiCol_ResizeGripActive]     = ImVec4(0.30f, 0.65f, 0.90f, 1.00f);
    colors[ImGuiCol_Tab]                  = ImVec4(0.14f, 0.14f, 0.17f, 1.00f);
    colors[ImGuiCol_TabHovered]           = ImVec4(0.30f, 0.65f, 0.90f, 0.60f);
    colors[ImGuiCol_TabSelected]          = ImVec4(0.20f, 0.45f, 0.70f, 1.00f);
    colors[ImGuiCol_TabDimmed]            = ImVec4(0.11f, 0.11f, 0.13f, 1.00f);
    colors[ImGuiCol_TabDimmedSelected]    = ImVec4(0.18f, 0.35f, 0.55f, 1.00f);
    colors[ImGuiCol_DockingPreview]       = ImVec4(0.30f, 0.65f, 0.90f, 0.50f);
    colors[ImGuiCol_DockingEmptyBg]       = ImVec4(0.09f, 0.09f, 0.11f, 1.00f);
    colors[ImGuiCol_TextSelectedBg]       = ImVec4(0.30f, 0.65f, 0.90f, 0.35f);
    colors[ImGuiCol_NavHighlight]         = ImVec4(0.30f, 0.65f, 0.90f, 1.00f);

    style.ScaleAllSizes(scale);

    spdlog::info("DPI scale applied: {:.2f}", scale);
}
