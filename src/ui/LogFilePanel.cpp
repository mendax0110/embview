#include "ui/LogFilePanel.h"

#include "core/LogFileManager.h"

#include <imgui.h>

#include <chrono>
#include <filesystem>

using namespace embview::ui;

LogFilePanel::LogFilePanel(std::shared_ptr<core::LogFileManager> logMgr)
    : m_logMgr(std::move(logMgr))
{
}

LogFilePanel::~LogFilePanel() = default;

void LogFilePanel::render(bool& open)
{
    if (!ImGui::Begin("Log Files", &open))
    {
        ImGui::End();
        return;
    }

    auto currentLog = m_logMgr->currentLogPath();
    ImGui::Text("Active: %s", currentLog.filename().string().c_str());
    ImGui::Separator();

    ImGui::SliderInt("Max age (days)", &m_deleteAgeDays, 1, 365);
    ImGui::SameLine();
    if (ImGui::Button("Purge Old"))
    {
        m_logMgr->deleteOlderThan(std::chrono::hours(m_deleteAgeDays * 24));
    }

    ImGui::Separator();

    auto logs = m_logMgr->listLogs();

    if (ImGui::BeginTable("LogFiles", 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable))
    {
        ImGui::TableSetupColumn("File");
        ImGui::TableSetupColumn("Size");
        ImGui::TableSetupColumn("Action");
        ImGui::TableHeadersRow();

        for (const auto& logPath : logs)
        {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();

            bool isCurrent = (logPath == currentLog);
            if (isCurrent)
            {
                ImGui::TextColored(ImVec4(0.3f, 0.85f, 0.45f, 1.0f), "%s",
                                   logPath.filename().string().c_str());
            }
            else
            {
                ImGui::TextUnformatted(logPath.filename().string().c_str());
            }

            ImGui::TableNextColumn();
            std::error_code ec;
            auto size = std::filesystem::file_size(logPath, ec);
            if (!ec)
            {
                if (size >= 1024 * 1024)
                {
                    ImGui::Text("%.1f MB", static_cast<double>(size) / (1024.0 * 1024.0));
                }
                else
                {
                    ImGui::Text("%.1f KB", static_cast<double>(size) / 1024.0);
                }
            }

            ImGui::TableNextColumn();
            if (isCurrent)
            {
                ImGui::TextDisabled("(active)");
            }
            else
            {
                ImGui::PushID(logPath.string().c_str());
                if (ImGui::SmallButton("Delete"))
                {
                    m_logMgr->deleteLog(logPath);
                }
                ImGui::PopID();
            }
        }

        ImGui::EndTable();
    }

    ImGui::End();
}
