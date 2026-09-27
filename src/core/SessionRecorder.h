#pragma once

#include "core/Protocol.h"

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <string>
#include <vector>

/**
 * @namespace embview::core Core support.
 */
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
        /**
         * @brief Create an empty session recorder.
         */
        SessionRecorder() = default;

        /**
         * @brief Destroy the recorder and close any active file.
         */
        ~SessionRecorder();
        SessionRecorder(const SessionRecorder&) = delete;
        SessionRecorder& operator=(const SessionRecorder&) = delete;

        /**
         * @brief Start Recording.
         * @param path
         * @return True on success.
         */
        bool startRecording(const std::filesystem::path& path);

        /**
         * @brief Record Frame.
         * @param frame
         */
        void recordFrame(const DataFrame& frame);

        /**
         * @brief Stop Recording.
         */
        void stopRecording();

        /**
         * @brief Check whether Recording.
         * @return True when the condition holds.
         */
        bool isRecording() const;

        /**
         * @brief Load Session.
         * @param path
         */
        static std::vector<DataFrame> loadSession(const std::filesystem::path& path);

        /**
         * @brief Recorded Frame Count.
         * @return The result.
         */
        std::size_t recordedFrameCount() const;

    private:
        mutable std::mutex m_mutex;
        std::ofstream m_file;
        bool m_recording = false;
        std::size_t m_frameCount = 0;
    };
} // namespace embview::core
