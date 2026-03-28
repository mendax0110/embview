#pragma once

#include <chrono>
#include <filesystem>
#include <memory>
#include <vector>

namespace spdlog { namespace sinks { class sink; } }

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
        explicit LogFileManager(std::filesystem::path logDir = "logs");
        ~LogFileManager();

        /// @brief Create log directory, set up rotating file sink, register on default logger.
        void init();

        /// @brief Get the path to the currently active log file.
        std::filesystem::path currentLogPath() const;

        /// @brief List all .log files in the log directory, sorted newest first.
        std::vector<std::filesystem::path> listLogs() const;

        /// @brief Delete a specific log file.
        void deleteLog(const std::filesystem::path& path);

        /// @brief Delete all log files older than the given age.
        void deleteOlderThan(std::chrono::hours maxAge);

    private:
        std::filesystem::path m_logDir;
        std::filesystem::path m_currentLog;
        std::shared_ptr<spdlog::sinks::sink> m_fileSink;
    };
} // namespace embview::core
