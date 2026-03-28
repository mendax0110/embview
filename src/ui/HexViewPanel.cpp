#include "ui/HexViewPanel.h"

#include "core/RawDataBuffer.h"

#include <imgui.h>

#include <algorithm>
#include <cstdio>

using namespace embview::ui;

HexViewPanel::HexViewPanel(std::shared_ptr<core::RawDataBuffer> rawBuffer)
    : m_rawBuffer(std::move(rawBuffer))
{
}

HexViewPanel::~HexViewPanel() = default;

void HexViewPanel::render(bool& open)
{
    if (!ImGui::Begin("Hex View", &open))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button(m_paused ? "Resume" : "Pause"))
    {
        m_paused = !m_paused;
    }
    ImGui::SameLine();
    if (ImGui::Button("Clear"))
    {
        m_rawBuffer->clear();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_autoScroll);
    ImGui::SameLine();
    ImGui::Text("(%zu bytes)", m_rawBuffer->size());

    ImGui::Separator();

    static std::vector<uint8_t> snapshot;
    if (!m_paused)
    {
        snapshot = m_rawBuffer->snapshot();
    }

    ImGui::BeginChild("HexScroll", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

    // 16 bytes per row
    constexpr int BYTES_PER_ROW = 16;
    std::size_t rowCount = (snapshot.size() + BYTES_PER_ROW - 1) / BYTES_PER_ROW;

    ImGuiListClipper clipper;
    clipper.Begin(static_cast<int>(rowCount));

    while (clipper.Step())
    {
        for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; ++row)
        {
            std::size_t offset = static_cast<std::size_t>(row) * BYTES_PER_ROW;
            std::size_t count = std::min(static_cast<std::size_t>(BYTES_PER_ROW),
                                          snapshot.size() - offset);

            // Address
            char line[128];
            int pos = std::snprintf(line, sizeof(line), "%06zX  ", offset);

            // Hex bytes
            for (std::size_t i = 0; i < BYTES_PER_ROW; ++i)
            {
                if (i < count)
                {
                    pos += std::snprintf(line + pos, sizeof(line) - static_cast<std::size_t>(pos),
                                          "%02X ", snapshot[offset + i]);
                }
                else
                {
                    pos += std::snprintf(line + pos, sizeof(line) - static_cast<std::size_t>(pos), "   ");
                }
                if (i == 7)
                {
                    pos += std::snprintf(line + pos, sizeof(line) - static_cast<std::size_t>(pos), " ");
                }
            }

            // ASCII
            pos += std::snprintf(line + pos, sizeof(line) - static_cast<std::size_t>(pos), " |");
            for (std::size_t i = 0; i < count; ++i)
            {
                uint8_t b = snapshot[offset + i];
                char c = (b >= 0x20 && b <= 0x7E) ? static_cast<char>(b) : '.';
                line[pos++] = c;
            }
            line[pos++] = '|';
            line[pos] = '\0';

            ImGui::TextUnformatted(line);
        }
    }

    if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::End();
}
