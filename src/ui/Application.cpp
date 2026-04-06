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
#include <functional>

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

    auto setUiMode = [app = std::ref(*this)](const ColorMode mode)
    {
        app.get().setUiMode(mode);
    };
    m_mainWindow = std::make_unique<MainWindow>(m_dataStore, m_deviceMgr, std::move(logFileMgr), std::move(setUiMode));

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

            if (m_pendingColorMode.has_value())
            {
                m_colorMode = m_pendingColorMode.value();
                m_pendingColorMode.reset();
                applyDpiScale(m_currentDpiScale);
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

    appTheme::apply(m_colorMode, scale);

    spdlog::info("DPI scale applied: {:.2f}", scale);
}

void Application::setUiMode(const ColorMode mode)
{
    m_pendingColorMode = mode;
}

ColorMode Application::getUiMode() const
{
    return m_colorMode;
}