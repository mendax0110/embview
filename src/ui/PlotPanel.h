#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

struct ImVec4;

namespace embview::core
{
    class DataStore;
    class TriggerEngine;
    class ExpressionEval;
    class SessionRecorder;
} // namespace embview::core

namespace embview::ui
{
    /**
     * @brief Annotation marker placed on the plot at a specific timestamp.
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
        ~PlotPanel();

        void render(bool& open);

    private:
        void exportCsv();

        struct ChannelConfig
        {
            std::string name;
            float color[4] = {0.0f, 0.0f, 0.0f, 0.0f};
            bool visible = true;
            std::string expression; // If non-empty, applies transform
        };

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
