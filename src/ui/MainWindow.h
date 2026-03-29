#pragma once

#include <memory>

namespace embview::core
{
    class DataStore;
    class DeviceManager;
    class LogFileManager;
    class TriggerEngine;
    class ExpressionEval;
    class SessionRecorder;
    class ConfigManager;
    class RawDataBuffer;
} // namespace embview::core

namespace embview::ui
{
    class ConnectionPanel;
    class PlotPanel;
    class LogPanel;
    class StatsPanel;
    class CommandPanel;
    class NumberConverterPanel;
    class LogFilePanel;
    class HexViewPanel;
    class RecorderPanel;
    class TriggerPanel;
    class FftPanel;

    /**
    * @brief Main window with menu bar, docking layout, and child panels.
    */
    class MainWindow
    {
    public:
        MainWindow(std::shared_ptr<core::DataStore> dataStore,
                   std::shared_ptr<core::DeviceManager> deviceMgr,
                   std::shared_ptr<core::LogFileManager> logFileMgr);
        ~MainWindow();

        void render();
        bool shouldClose() const;
        void requestLayoutRebuild();

    private:
        void renderMenuBar();
        void renderDockSpace();
        void renderStatusBar();

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
        float m_lastLayoutWidth = 0.0f;
        float m_lastLayoutHeight = 0.0f;

        double m_lastFpsTime = 0.0;
        std::size_t m_lastFrameCount = 0;
        float m_dataFps = 0.0f;

        bool m_showAboutPopup = false;
    };
} // namespace embview::ui
