#include "ui/StatsPanel.h"

#include "core/DataStore.h"

#include <imgui.h>

#include <algorithm>
#include <cmath>
#include <numeric>

using namespace embview::ui;

StatsPanel::StatsPanel(std::shared_ptr<core::DataStore> dataStore)
    : m_dataStore(std::move(dataStore))
{
}

StatsPanel::~StatsPanel() = default;

void StatsPanel::render(bool& open) const
{
    if (!ImGui::Begin("Statistics", &open))
    {
        ImGui::End();
        return;
    }

    auto channels = m_dataStore->getActiveChannels();
    std::ranges::sort(channels.begin(), channels.end());

    if (channels.empty())
    {
        ImGui::TextDisabled("No data available");
        ImGui::End();
        return;
    }

    if (ImGui::BeginTable("Stats", 7,
        ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Channel");
        ImGui::TableSetupColumn("Count");
        ImGui::TableSetupColumn("Min");
        ImGui::TableSetupColumn("Max");
        ImGui::TableSetupColumn("Mean");
        ImGui::TableSetupColumn("StdDev");
        ImGui::TableSetupColumn("Latest");
        ImGui::TableHeadersRow();

        for (const uint16_t ch : channels)
        {
            auto frames = m_dataStore->getChannel(ch);
            if (frames.empty())
            {
                continue;
            }

            double minVal = frames[0].value;
            double maxVal = frames[0].value;
            double sum = 0.0;

            for (const auto& f : frames)
            {
                minVal = std::min(minVal, f.value);
                maxVal = std::max(maxVal, f.value);
                sum += f.value;
            }

            const double mean = sum / static_cast<double>(frames.size());

            double sqDiffSum = 0.0;
            for (const auto& f : frames)
            {
                const double diff = f.value - mean;
                sqDiffSum += diff * diff;
            }
            const double stddev = std::sqrt(sqDiffSum / static_cast<double>(frames.size()));

            const auto devIdx = static_cast<uint8_t>(ch / 256);
            const auto rawCh = static_cast<uint8_t>(ch % 256);

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (devIdx > 0)
            {
                ImGui::Text("D%d/Ch%d", devIdx, rawCh);
            }
            else
            {
                ImGui::Text("Ch %d", rawCh);
            }
            ImGui::TableNextColumn(); ImGui::Text("%zu", frames.size());
            ImGui::TableNextColumn(); ImGui::Text("%.4f", minVal);
            ImGui::TableNextColumn(); ImGui::Text("%.4f", maxVal);
            ImGui::TableNextColumn(); ImGui::Text("%.4f", mean);
            ImGui::TableNextColumn(); ImGui::Text("%.4f", stddev);
            ImGui::TableNextColumn(); ImGui::Text("%.4f", frames.back().value);
        }

        ImGui::EndTable();
    }

    ImGui::End();
}
