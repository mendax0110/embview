#pragma once

#include "core/Protocol.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

namespace embview::core
{
    /**
     * @brief Records DataFrames to a binary file and replays them.
     *
     * File format: sequence of [uint16_t channel][double timestamp][double value]
     * (18 bytes per frame, little-endian).
     */
    class SessionRecorder
    {
    public:
        SessionRecorder() = default;
        ~SessionRecorder();

        SessionRecorder(const SessionRecorder&) = delete;
        SessionRecorder& operator=(const SessionRecorder&) = delete;

        bool startRecording(const std::filesystem::path& path);

        void recordFrame(const DataFrame& frame);

        void stopRecording();

        bool isRecording() const;

        std::vector<DataFrame> loadSession(const std::filesystem::path& path);

        std::size_t recordedFrameCount() const;

    private:
        mutable std::mutex m_mutex;
        std::ofstream m_file;
        bool m_recording = false;
        std::size_t m_frameCount = 0;
    };
} // namespace embview::core
