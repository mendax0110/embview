#pragma once

#include <chrono>
#include <filesystem>
#include <memory>
#include <vector>

/**
 * @namespace spdlog::sinks
 */
namespace spdlog::sinks
{
    class sink;
}

/**
 * @namespace embview::core Core support.
 */
namespace embview::core
{
    /**
     * @brief Manages log files on disk with rotation and cleanup.
     *
     * Creates a rotating file sink for spdlog and provides utilities
     * to list and delete old log files from the log directory.
     */
    class LogFileManager
    {
    public:
        /**
         * @brief Create a log manager for the given directory.
         * @param logDir Directory used for rotating log files.
         */
        explicit LogFileManager(std::filesystem::path logDir = "logs");

        /**
         * @brief Destroy the log manager and release the sink.
         */
        ~LogFileManager();

        /**
         * @brief Initialize the log directory and default logger sink.
         */
        void init();

        /**
         * @brief Get the current active log file path.
         * @return Path to the active log file.
         */
        [[nodiscard]] std::filesystem::path currentLogPath() const;

        /**
         * @brief List all log files in the directory.
         * @return Log files sorted newest first.
         */
        [[nodiscard]] std::vector<std::filesystem::path> listLogs() const;

        /**
         * @brief Delete a specific log file.
         * @param path
         */
        void deleteLog(const std::filesystem::path& path) const;

        /**
         * @brief Delete all log files older than the given age.
         * @param maxAge
         */
        void deleteOlderThan(std::chrono::hours maxAge) const;

    private:
        std::filesystem::path m_logDir;
        std::filesystem::path m_currentLog;
        std::shared_ptr<spdlog::sinks::sink> m_fileSink;
    };
} // namespace embview::core
