#pragma once

#include <memory>
#include <string>

struct GLFWwindow;

namespace embview::core
{
    class DataStore;
    class DeviceManager;
    class LogFileManager;
} // namespace embview::core

namespace embview::ui
{
    class MainWindow;

    /**
    * @brief Top-level application managing the ImGui/GLFW lifecycle.
    *
    * Initializes the window, OpenGL context, Dear ImGui, and ImPlot.
    * Owns the main window and shared core objects.
    */
    class Application
    {
    public:
        Application();
        ~Application();

        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        bool init(std::shared_ptr<core::LogFileManager> logFileMgr,
                  const std::string& title = "embview", int width = 1280, int height = 720);

        void run();
        void shutdown();

    private:
        void applyDpiScale(float scale);

        GLFWwindow* m_window = nullptr;
        std::unique_ptr<MainWindow> m_mainWindow;
        std::shared_ptr<core::DataStore> m_dataStore;
        std::shared_ptr<core::DeviceManager> m_deviceMgr;
        bool m_initialized = false;
        float m_currentDpiScale = 1.0f;
    };
} // namespace embview::ui
