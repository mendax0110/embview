#pragma once

#include <functional>
#include <memory>

#include "AppTheme.h"

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

    /**
     * @brief Trigger Engine.
     * \class TriggerEngine
     */
    class TriggerEngine;

    /**
     * @brief Expression Eval.
     * \class ExpressionEval
     */
    class ExpressionEval;

    /**
     * @brief Session Recorder.
     * \class SessionRecorder
     */
    class SessionRecorder;

    /**
     * @brief Config Manager.
     * \class ConfigManager
     */
    class ConfigManager;

    /**
     * @brief Raw Data Buffer.
     * \class RawDataBuffer
     */
    class RawDataBuffer;
} // namespace embview::core

/**
 * @namespace embview::ui Ui support.
 */
namespace embview::ui
{
    /**
     * @brief Connection Panel.
     * \class ConnectionPanel
     */
    class ConnectionPanel;

    /**
     * @brief Plot Panel.
     * \class PlotPanel
     */
    class PlotPanel;

    /**
     * @brief Log Panel.
     * \class LogPanel
     */
    class LogPanel;

    /**
     * @brief Stats Panel.
     * \class StatsPanel
     */
    class StatsPanel;

    /**
     * @brief Command Panel.
     * \class CommandPanel
     */
    class CommandPanel;

    /**
     * @brief Number Converter Panel.
     * \class NumberConverterPanel
     */
    class NumberConverterPanel;

    /**
     * @brief Log File Panel.
     * \class LogFilePanel
     */
    class LogFilePanel;

    /**
     * @brief Hex View Panel.
     * \class HexViewPanel
     */
    class HexViewPanel;

    /**
     * @brief Recorder Panel.
     * \class RecorderPanel
     */
    class RecorderPanel;

    /**
     * @brief Trigger Panel.
     * \class TriggerPanel
     */
    class TriggerPanel;

    /**
     * @brief Fft Panel.
     * \class FftPanel
     */
    class FftPanel;

    /**
    * @brief Main window with menu bar, docking layout, and child panels.
    */
    class MainWindow
    {
    public:
        /**
         * @brief Create the main application window and attach child panels.
         * @param dataStore Shared data store for live samples.
         * @param deviceMgr Shared device manager.
         * @param logFileMgr Shared log file manager.
         * @param setUiMode Callback for UI theme changes.
         */
        MainWindow(std::shared_ptr<core::DataStore> dataStore,
                  std::shared_ptr<core::DeviceManager> deviceMgr,
                  std::shared_ptr<core::LogFileManager> logFileMgr,
                  std::function<void(ColorMode)> setUiMode);
        /**
         * @brief Destroy the main window.
         */
        ~MainWindow();

        /**
         * @brief Render the main window frame.
         */
        void render();

        /**
         * @brief Check whether the main window should close.
         * @return true when the close request is pending.
         */
        [[nodiscard]] bool shouldClose() const;

        /**
         * @brief Request Layout Rebuild.
         */
        void requestLayoutRebuild();

    private:
        /**
         * @brief Render Menu Bar.
         */
        void renderMenuBar();

        /**
         * @brief
         */
        void renderDockSpace();

        /**
         * @brief Render Status Bar.
         */
        void renderStatusBar();

        /**
         * @brief Build the default panel layout for the dock space.
         * @param dockspaceId ImGui dockspace identifier.
         */
        static void buildDefaultLayout(unsigned int dockspaceId);

        std::shared_ptr<core::DataStore> m_dataStore;
        std::shared_ptr<core::DeviceManager> m_deviceMgr;
        std::shared_ptr<core::TriggerEngine> m_triggerEngine;
        std::shared_ptr<core::ExpressionEval> m_exprEval;
        std::shared_ptr<core::SessionRecorder> m_recorder;
        std::shared_ptr<core::ConfigManager> m_configMgr;

        std::unique_ptr<ConnectionPanel> m_connectionPanel;
        std::unique_ptr<PlotPanel> m_plotPanel;
        std::unique_ptr<LogPanel> m_logPanel;
        std::unique_ptr<StatsPanel> m_statsPanel;
        std::unique_ptr<CommandPanel> m_commandPanel;
        std::unique_ptr<NumberConverterPanel> m_numberConverterPanel;
        std::unique_ptr<LogFilePanel> m_logFilePanel;
        std::unique_ptr<HexViewPanel> m_hexViewPanel;
        std::unique_ptr<RecorderPanel> m_recorderPanel;
        std::unique_ptr<TriggerPanel> m_triggerPanel;
        std::unique_ptr<FftPanel> m_fftPanel;

        /**
         * @brief Void.
         */
        std::function<void(ColorMode)> m_setUiMode;

        bool m_showConnectionPanel = true;
        bool m_showPlotPanel = true;
        bool m_showLogPanel = true;
        bool m_showStatsPanel = true;
        bool m_showCommandPanel = false;
        bool m_showNumberConverter = false;
        bool m_showLogFilePanel = false;
        bool m_showHexView = false;
        bool m_showRecorder = false;
        bool m_showTriggers = false;
        bool m_showFft = false;
        bool m_shouldClose = false;
        bool m_needsLayoutRebuild = true;
        bool m_autoScaleLayout = true;
        bool m_showColorModePanel = true;
        float m_lastLayoutWidth = 0.0f;
        float m_lastLayoutHeight = 0.0f;

        double m_lastFpsTime = 0.0;
        std::size_t m_lastFrameCount = 0;
        float m_dataFps = 0.0f;

        bool m_showAboutPopup = false;
    };
} // namespace embview::ui
