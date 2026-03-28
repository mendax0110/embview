#include "ui/LogPanel.h"

#include <imgui.h>
#include <spdlog/spdlog.h>

#include <mutex>
#include <vector>

using namespace embview::ui;

class LogPanel::ImGuiSink : public spdlog::sinks::base_sink<std::mutex>
{
public:
    std::vector<LogEntry> getEntries()
    {
        std::lock_guard lock(mutex_);
        return m_entries;
    }

    void clearEntries()
    {
        std::lock_guard lock(mutex_);
        m_entries.clear();
    }

protected:
    void sink_it_(const spdlog::details::log_msg& msg) override
    {
        spdlog::memory_buf_t formatted;
        formatter_->format(msg, formatted);

        LogEntry entry;
        entry.level = msg.level;
        entry.message = fmt::to_string(formatted);
        m_entries.push_back(std::move(entry));

        // Cap at 1000 entries
        if (m_entries.size() > 1000)
        {
            m_entries.erase(m_entries.begin());
        }
    }

    void flush_() override {}

private:
    std::vector<LogEntry> m_entries;
};

LogPanel::LogPanel()
    : m_sink(std::make_shared<ImGuiSink>())
{
    spdlog::default_logger()->sinks().push_back(m_sink);
}

LogPanel::~LogPanel()
{
    auto& sinks = spdlog::default_logger()->sinks();
    sinks.erase(std::remove(sinks.begin(), sinks.end(), m_sink), sinks.end());
}

void LogPanel::render(bool& open)
{
    if (!ImGui::Begin("Log", &open))
    {
        ImGui::End();
        return;
    }

    if (ImGui::Button("Clear"))
    {
        m_sink->clearEntries();
    }
    ImGui::SameLine();
    ImGui::Checkbox("Auto-scroll", &m_autoScroll);

    ImGui::Separator();

    ImGui::BeginChild("LogScroll", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar);

    auto entries = m_sink->getEntries();
    for (const auto& entry : entries)
    {
        ImVec4 color;
        switch (entry.level)
        {
            case spdlog::level::err:
            case spdlog::level::critical:
                color = ImVec4(1.0f, 0.3f, 0.3f, 1.0f);
                break;
            case spdlog::level::warn:
                color = ImVec4(1.0f, 1.0f, 0.3f, 1.0f);
                break;
            case spdlog::level::info:
                color = ImVec4(0.8f, 0.8f, 0.8f, 1.0f);
                break;
            default:
                color = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
                break;
        }

        ImGui::PushStyleColor(ImGuiCol_Text, color);
        ImGui::TextUnformatted(entry.message.c_str());
        ImGui::PopStyleColor();
    }

    if (m_autoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
    {
        ImGui::SetScrollHereY(1.0f);
    }

    ImGui::EndChild();
    ImGui::End();
}

std::shared_ptr<spdlog::sinks::sink> LogPanel::getSink() const
{
    return m_sink;
}
