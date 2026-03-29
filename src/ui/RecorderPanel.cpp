#include "ui/RecorderPanel.h"

#include "core/DataStore.h"
#include "core/SessionRecorder.h"

#include <imgui.h>
#include <spdlog/spdlog.h>

#include <chrono>
#include <cstdio>
#include <filesystem>

using namespace embview::ui;

RecorderPanel::RecorderPanel(std::shared_ptr<core::DataStore> dataStore,
                             std::shared_ptr<core::SessionRecorder> recorder)
    : m_dataStore(std::move(dataStore))
    , m_recorder(std::move(recorder))
{
    // Generate default filename with timestamp
    const auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif
    std::strftime(m_filename, sizeof(m_filename), "session_%Y%m%d_%H%M%S.emb", &tm);
}

RecorderPanel::~RecorderPanel() = default;

void RecorderPanel::render(bool& open)
{
    if (!ImGui::Begin("Recorder", &open))
    {
        ImGui::End();
        return;
    }

    // Recording section
    ImGui::Text("Recording");
    ImGui::Separator();

    ImGui::InputText("Filename", m_filename, sizeof(m_filename));

    const bool recording = m_recorder->isRecording();

    if (recording)
    {
        if (ImGui::Button("Stop Recording"))
        {
            m_recorder->stopRecording();
        }
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "REC (%zu frames)",
                           m_recorder->recordedFrameCount());
    }
    else
    {
        if (ImGui::Button("Start Recording"))
        {
            m_recorder->startRecording(m_filename);
        }
    }

    ImGui::Spacing();
    ImGui::Text("Playback");
    ImGui::Separator();

    static char loadPath[256] = "";
    ImGui::InputText("Load File", loadPath, sizeof(loadPath));

    if (ImGui::Button("Load & Replay"))
    {
        try
        {
            const auto frames = core::SessionRecorder::loadSession(loadPath);
            if (!frames.empty())
            {
                m_dataStore->clear();
                for (const auto& frame : frames)
                {
                    m_dataStore->push(frame);
                }
                spdlog::info("Replayed {} frames from {}", frames.size(), loadPath);
            }
        }
        catch (const std::exception& e)
        {
            spdlog::error("Failed to load session: {}", e.what());
        }
    }

    // List .emb files in current directory
    ImGui::Spacing();
    ImGui::Text("Available Sessions:");
    ImGui::Separator();

    try
    {
        std::error_code ec;
        for (const auto& entry : std::filesystem::directory_iterator(".", ec))
        {
            if (entry.is_regular_file() && entry.path().extension() == ".emb")
            {
                auto name = entry.path().filename().string();
                if (ImGui::Selectable(name.c_str()))
                {
                    std::snprintf(loadPath, sizeof(loadPath), "%s", name.c_str());
                }
            }
        }
    }
    catch (const std::exception& e)
    {
        ImGui::TextDisabled("Error listing files: %s", e.what());
    }

    ImGui::End();
}
