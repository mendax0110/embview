#include "core/SessionRecorder.h"

#include <spdlog/spdlog.h>

#include <cstring>

using namespace embview::core;

SessionRecorder::~SessionRecorder()
{
    if (m_recording)
    {
        stopRecording();
    }
}

bool SessionRecorder::startRecording(const std::filesystem::path& path)
{
    std::lock_guard lock(m_mutex);
    if (m_recording)
    {
        return false;
    }

    m_file.open(path, std::ios::binary | std::ios::trunc);
    if (!m_file.is_open())
    {
        spdlog::error("Failed to open recording file: {}", path.string());
        return false;
    }

    m_recording = true;
    m_frameCount = 0;
    spdlog::info("Recording started: {}", path.string());
    return true;
}

void SessionRecorder::recordFrame(const DataFrame& frame)
{
    std::lock_guard lock(m_mutex);
    if (!m_recording || !m_file.is_open())
    {
        return;
    }

    m_file.write(reinterpret_cast<const char*>(&frame.channel), sizeof(frame.channel));
    m_file.write(reinterpret_cast<const char*>(&frame.timestamp), sizeof(frame.timestamp));
    m_file.write(reinterpret_cast<const char*>(&frame.value), sizeof(frame.value));
    ++m_frameCount;
}

void SessionRecorder::stopRecording()
{
    std::lock_guard lock(m_mutex);
    if (!m_recording)
    {
        return;
    }

    m_file.close();
    m_recording = false;
    spdlog::info("Recording stopped ({} frames)", m_frameCount);
}

bool SessionRecorder::isRecording() const
{
    std::lock_guard lock(m_mutex);
    return m_recording;
}

std::vector<DataFrame> SessionRecorder::loadSession(const std::filesystem::path& path)
{
    std::vector<DataFrame> result;

    std::ifstream file(path, std::ios::binary);
    if (!file.is_open())
    {
        spdlog::error("Failed to open session file: {}", path.string());
        return result;
    }

    while (file.good())
    {
        DataFrame frame{};
        file.read(reinterpret_cast<char*>(&frame.channel), sizeof(frame.channel));
        file.read(reinterpret_cast<char*>(&frame.timestamp), sizeof(frame.timestamp));
        file.read(reinterpret_cast<char*>(&frame.value), sizeof(frame.value));

        if (file.gcount() == sizeof(frame.value))
        {
            result.push_back(frame);
        }
    }

    spdlog::info("Loaded session: {} ({} frames)", path.string(), result.size());
    return result;
}

std::size_t SessionRecorder::recordedFrameCount() const
{
    std::lock_guard lock(m_mutex);
    return m_frameCount;
}
