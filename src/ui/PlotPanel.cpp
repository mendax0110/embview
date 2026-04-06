#include "ui/PlotPanel.h"

#include "core/DataStore.h"
#include "core/ExpressionEval.h"
#include "core/SessionRecorder.h"
#include "core/TriggerEngine.h"

#include <imgui.h>
#include <implot.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <string>
#include <vector>

using namespace embview::ui;

PlotPanel::PlotPanel(std::shared_ptr<core::DataStore> dataStore,
                     std::shared_ptr<core::TriggerEngine> triggerEngine,
                     std::shared_ptr<core::ExpressionEval> exprEval,
                     std::shared_ptr<core::SessionRecorder> recorder)
    : m_dataStore(std::move(dataStore))
    , m_triggerEngine(std::move(triggerEngine))
    , m_exprEval(std::move(exprEval))
    , m_recorder(std::move(recorder))
{
}

PlotPanel::~PlotPanel() = default;

PlotPanel::ChannelConfig& PlotPanel::getConfig(const uint16_t channel)
{
    const auto it = m_channelConfigs.find(channel);
    if (it == m_channelConfigs.end())
    {
        ChannelConfig cfg;
        const auto devIdx = static_cast<uint8_t>(channel / 256);
        const auto rawCh = static_cast<uint8_t>(channel % 256);
        if (devIdx > 0)
        {
            cfg.name = "D" + std::to_string(devIdx) + "/Ch" + std::to_string(rawCh);
        }
        else
        {
            cfg.name = "Ch " + std::to_string(rawCh);
        }
        m_channelConfigs[channel] = cfg;
        return m_channelConfigs[channel];
    }
    return it->second;
}

void PlotPanel::render(bool& open)
{
    if (!ImGui::Begin("Plot", &open))
    {
        ImGui::End();
        return;
    }

    // Controls
    ImGui::Checkbox("Pause", &m_paused);
    ImGui::SameLine();
    ImGui::SliderFloat("Time Window", &m_timeWindow, 1.0f, 60.0f, "%.1f s");
    ImGui::SameLine();
    if (ImGui::Button("Clear"))
    {
        m_dataStore->clear();
    }
    ImGui::SameLine();
    if (ImGui::Button("Export CSV"))
    {
        exportCsv();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Channel Settings", &m_showChannelSettings);

    auto channels = m_dataStore->getActiveChannels();
    std::ranges::sort(channels.begin(), channels.end());

    // Record frames if recording
    if (m_recorder && m_recorder->isRecording() && !m_paused)
    {
        for (const uint16_t ch : channels)
        {
            auto frames = m_dataStore->getChannel(ch);
            for (const auto& f : frames)
            {
                m_recorder->recordFrame(f);
            }
        }
    }

    // Evaluate triggers
    if (m_triggerEngine && !m_paused)
    {
        for (const uint16_t ch : channels)
        {
            auto frames = m_dataStore->getChannel(ch);
            if (!frames.empty())
            {
                m_triggerEngine->evaluate(frames.back());
            }
        }
    }

    // Channel configuration section
    if (m_showChannelSettings)
    {
        if (channels.empty())
        {
            ImGui::TextDisabled("No channels yet -- connect a device to see channel settings");
        }
    }
    if (m_showChannelSettings && !channels.empty())
    {
        if (ImGui::BeginChild("ChSettings", ImVec2(0, 150), ImGuiChildFlags_Border | ImGuiChildFlags_ResizeY))
        {
            for (const uint16_t ch : channels)
            {
                auto&[name, color, visible, expression] = getConfig(ch);
                ImGui::PushID(ch);

                ImGui::Checkbox("##vis", &visible);
                ImGui::SameLine();

                char nameBuf[64];
                std::snprintf(nameBuf, sizeof(nameBuf), "%s", name.c_str());
                ImGui::SetNextItemWidth(120.0f);
                if (ImGui::InputText("##name", nameBuf, sizeof(nameBuf)))
                {
                    name = nameBuf;
                }
                ImGui::SameLine();

                bool useCustomColor = (color[3] > 0.0f);
                if (ImGui::Checkbox("Color", &useCustomColor))
                {
                    if (!useCustomColor)
                    {
                        color[3] = 0.0f;
                    }
                    else
                    {
                        auto col = ImPlot::GetColormapColor(ch);
                        color[0] = col.x;
                        color[1] = col.y;
                        color[2] = col.z;
                        color[3] = col.w;
                    }
                }
                if (useCustomColor)
                {
                    ImGui::SameLine();
                    ImGui::ColorEdit4("##color", color, ImGuiColorEditFlags_NoInputs);
                }

                // Expression transform
                ImGui::SameLine();
                char exprBuf[128];
                std::snprintf(exprBuf, sizeof(exprBuf), "%s", expression.c_str());
                ImGui::SetNextItemWidth(200.0f);
                if (ImGui::InputText("Expr", exprBuf, sizeof(exprBuf)))
                {
                    expression = exprBuf;
                }
                if (ImGui::IsItemHovered())
                {
                    ImGui::SetTooltip(
                        "Expression transform applied to plotted values.\n"
                        "Variables: x (raw value), t (timestamp), ch0, ch1, ...\n"
                        "Functions: abs, sqrt, sin, cos, tan, log, log10, exp, floor, ceil, round\n"
                        "Constants: pi, e\n"
                        "Examples:\n"
                        "  x * 3.3 / 4096   (ADC to voltage)\n"
                        "  ch0 - ch1         (channel difference)\n"
                        "  abs(x)            (absolute value)");
                }

                // Show expression validation feedback
                if (!expression.empty() && m_exprEval)
                {
                    m_exprEval->setVariable("x", 0.0);
                    m_exprEval->setVariable("t", 0.0);
                    const auto rawCh = static_cast<uint8_t>(ch % 256);
                    m_exprEval->setVariable("ch" + std::to_string(rawCh), 0.0);
                    m_exprEval->evaluate(expression);
                    if (m_exprEval->hasError())
                    {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "ERR");
                        if (ImGui::IsItemHovered())
                        {
                            ImGui::SetTooltip("Expression error: %s", m_exprEval->errorMessage().c_str());
                        }
                    }
                    else
                    {
                        ImGui::SameLine();
                        ImGui::TextColored(ImVec4(0.3f, 0.85f, 0.45f, 1.0f), "OK");
                    }
                }

                ImGui::PopID();
            }
        }
        ImGui::EndChild();
    }

    // Marker input
    ImGui::SetNextItemWidth(200.0f);
    ImGui::InputText("##marker", m_markerText, sizeof(m_markerText));
    ImGui::SameLine();
    if (ImGui::Button("Add Marker"))
    {
        if (m_markerText[0] != '\0' && !channels.empty())
        {
            const auto frames = m_dataStore->getChannel(channels.front());
            const double ts = frames.empty() ? 0.0 : frames.back().timestamp;
            m_markers.push_back({ts, std::string(m_markerText)});
            m_markerText[0] = '\0';
        }
    }

    // Plot
    if (ImPlot::BeginPlot("Data", ImVec2(-1, -1), ImPlotFlags_NoTitle))
    {
        ImPlot::SetupAxes("Time", "Value");

        if (!m_paused && !channels.empty())
        {
            ImPlot::SetupAxisLimits(ImAxis_X1, 0, m_timeWindow, ImPlotCond_Always);
            ImPlot::SetupAxisLimits(ImAxis_Y1, 0, 1, ImPlotCond_Once);
        }

        for (const uint16_t ch : channels)
        {
            auto&[name, color, visible, expression] = getConfig(ch);
            if (!visible)
            {
                continue;
            }

            auto frames = m_dataStore->getChannel(ch);
            if (frames.empty())
            {
                continue;
            }

            std::vector<double> xs, ys;
            xs.reserve(frames.size());
            ys.reserve(frames.size());

            const bool hasExpr = !expression.empty() && m_exprEval;

            // Pre-load all channel latest values so cross-channel expressions work
            if (hasExpr)
            {
                for (const uint16_t otherCh : channels)
                {
                    auto otherFrames = m_dataStore->getChannel(otherCh);
                    if (!otherFrames.empty())
                    {
                        const auto rawCh = static_cast<uint8_t>(otherCh % 256);
                        m_exprEval->setVariable("ch" + std::to_string(rawCh), otherFrames.back().value);
                        m_exprEval->setChannelValue(otherCh, otherFrames.back().value);
                    }
                }
            }

            bool exprErrorLogged = false;

            for (const auto& f : frames)
            {
                xs.push_back(f.timestamp);

                if (hasExpr)
                {
                    auto rawCh = static_cast<uint8_t>(ch % 256);
                    m_exprEval->setVariable("ch" + std::to_string(rawCh), f.value);
                    m_exprEval->setChannelValue(ch, f.value);
                    m_exprEval->setVariable("x", f.value);
                    m_exprEval->setVariable("t", f.timestamp);

                    double result = m_exprEval->evaluate(expression);
                    if (m_exprEval->hasError())
                    {
                        if (!exprErrorLogged)
                        {
                            spdlog::debug("Expression error on ch{}: {}", rawCh,
                                          m_exprEval->errorMessage());
                            exprErrorLogged = true;
                        }
                        ys.push_back(f.value);
                    }
                    else
                    {
                        ys.push_back(result);
                    }
                }
                else
                {
                    ys.push_back(f.value);
                }
            }

            if (color[3] > 0.0f)
            {
                ImPlot::SetNextLineStyle(ImVec4(color[0], color[1], color[2], color[3]));
            }

            ImPlot::PlotLine(name.c_str(), xs.data(), ys.data(), static_cast<int>(xs.size()));
        }

        // Draw markers
        for (const auto&[timestamp, text] : m_markers)
        {
            double x = timestamp;
            ImPlot::DragLineX(0, &x, ImVec4(1, 1, 0, 0.7f), 1.0f);
            ImPlot::Annotation(timestamp, 0, ImVec4(1, 1, 0.3f, 1), ImVec2(5, -5), true,
                                "%s", text.c_str());
        }

        // Draw trigger threshold lines
        if (m_triggerEngine)
        {
            const auto& triggers = m_triggerEngine->triggers();
            for (std::size_t i = 0; i < triggers.size(); ++i)
            {
                auto& t = triggers[i];
                if (!t.enabled)
                {
                    continue;
                }
                double y = t.threshold;
                ImPlot::DragLineY(static_cast<int>(i + 1000), &y, ImVec4(1, 0.3f, 0.3f, 0.6f), 1.0f);
            }
        }

        ImPlot::EndPlot();
    }

    ImGui::End();
}

void PlotPanel::exportCsv()
{
    try
    {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
#ifdef _WIN32
        localtime_s(&tm, &time);
#else
        localtime_r(&time, &tm);
#endif

        char filename[64];
        std::strftime(filename, sizeof(filename), "embview_export_%Y%m%d_%H%M%S.csv", &tm);

        std::ofstream file(filename);
        if (!file.is_open())
        {
            spdlog::error("Failed to create export file: {}", filename);
            return;
        }

        file << "timestamp,device,channel,value\n";

        auto channels = m_dataStore->getActiveChannels();
        std::ranges::sort(channels.begin(), channels.end());

        for (uint16_t ch : channels)
        {
            auto frames = m_dataStore->getChannel(ch);
            auto devIdx = static_cast<uint8_t>(ch / 256);
            auto rawCh = static_cast<uint8_t>(ch % 256);

            for (const auto& f : frames)
            {
                file << f.timestamp << ","
                     << static_cast<int>(devIdx) << ","
                     << static_cast<int>(rawCh) << ","
                     << f.value << "\n";
            }
        }

        if (!file.good())
        {
            spdlog::error("Write error during CSV export to {}", filename);
            return;
        }

        spdlog::info("Exported data to {}", filename);
    }
    catch (const std::exception& e)
    {
        spdlog::error("CSV export failed: {}", e.what());
    }
}
