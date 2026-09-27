#pragma once

#include <memory>
#include <optional>
#include <string>
#include "AppTheme.h"

/**
 * @brief G L F Wwindow.
 * \struct GLFWwindow
 */
struct GLFWwindow;

/**
 * @namespace embview::core Core support.
 */
namespace embview::core
{
    /**
     * @brief Data Store.
     * \class DataStore
     */
    class DataStore;
    /**
     * @brief Device Manager.
     * \class DeviceManager
     */
    class DeviceManager;
    /**
     * @brief Log File Manager.
     * \class LogFileManager
     */
    class LogFileManager;
} // namespace embview::core

/**
 * @namespace embview::ui Ui support.
 */
namespace embview::ui
{
    /**
     * @brief Main Window.
     * \class MainWindow
     */
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
        /**
         * @brief Create the application shell.
         */
        Application();

        /**
         * @brief Destroy the application and release owned resources.
         */
        ~Application();
        Application(const Application&) = delete;
        Application& operator=(const Application&) = delete;

        /**
         * @brief Initialize the app and main window.
         * @param logFileMgr Shared log manager used by the UI.
         * @param title Window title.
         * @param width Window width.
         * @param height Window height.
         * @return true on success.
         */
        bool init(std::shared_ptr<core::LogFileManager> logFileMgr, const std::string& title = "embview", int width = 1280, int height = 720);

        /**
         * @brief Run the application event loop.
         */
        void run();

        /**
         * @brief Shut down the application.
         */
        void shutdown();

        /**
         * @brief Set the UI color mode.
         * @param mode Target color mode.
         */
        void setUiMode(ColorMode mode);

        /**
         * @brief Get the active UI color mode.
         * @return Current color mode.
         */
        [[nodiscard]] ColorMode getUiMode() const;

    private:
        /**
         * @brief Apply a DPI scaling factor.
         * @param scale DPI scale multiplier.
         */
        void applyDpiScale(float scale);

        GLFWwindow* m_window = nullptr;
        std::unique_ptr<MainWindow> m_mainWindow;
        std::shared_ptr<core::DataStore> m_dataStore;
        std::shared_ptr<core::DeviceManager> m_deviceMgr;
        bool m_initialized = false;
        float m_currentDpiScale = 1.0f;
        ColorMode m_colorMode = ColorMode::Dark;
        std::optional<ColorMode> m_pendingColorMode;
    };
} // namespace embview::ui
