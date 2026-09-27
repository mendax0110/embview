#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

/**
 * @brief Im Vec4.
 * \struct ImVec4
 */
struct ImVec4;

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
} // namespace embview::core

/**
 * @namespace embview::ui Ui support.
 */
namespace embview::ui
{
    /**
     * @brief Annotation marker placed on the plot at a specific timestamp. \struct PlotMarker
     */
    struct PlotMarker
    {
        double timestamp;
        std::string text;
    };

    /**
     * @brief Real-time data plotting panel using ImPlot.
     *
     * Displays per-channel time-series with scrolling X-axis
     * and auto-scaling Y-axis. Supports pause/resume, clearing data,
     * CSV export, per-channel configuration, annotations, trigger
     * highlights, and expression-based virtual channels.
     */
    class PlotPanel
    {
    public:
        PlotPanel(std::shared_ptr<core::DataStore> dataStore,
                  std::shared_ptr<core::TriggerEngine> triggerEngine,
                  std::shared_ptr<core::ExpressionEval> exprEval,
                  std::shared_ptr<core::SessionRecorder> recorder);
        /**
         * @brief Destroy the plot panel and release resources.
         */
        ~PlotPanel();

        /**
         * @brief Render the plot panel.
         * @param open Whether the panel is open.
         */
        void render(bool& open);

    private:
        /**
         * @brief Export plotted data to CSV.
         */
        void exportCsv();

        /**
         * @brief Channel display configuration. \struct ChannelConfig
         */
        struct ChannelConfig
        {
            std::string name;
            float color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
            bool visible = true;
            std::string expression; // If non-empty, applies transform
        };

        /**
         * @brief Get the display configuration for a channel.
         * @param channel Channel identifier.
         * @return Mutable channel configuration.
         */
        ChannelConfig& getConfig(uint16_t channel);

        std::shared_ptr<core::DataStore> m_dataStore;
        std::shared_ptr<core::TriggerEngine> m_triggerEngine;
        std::shared_ptr<core::ExpressionEval> m_exprEval;
        std::shared_ptr<core::SessionRecorder> m_recorder;
        std::unordered_map<uint16_t, ChannelConfig> m_channelConfigs;
        std::vector<PlotMarker> m_markers;
        bool m_paused = false;
        float m_timeWindow = 10.0f;
        bool m_showChannelSettings = false;
        char m_markerText[128] = "";
    };
} // namespace embview::ui
