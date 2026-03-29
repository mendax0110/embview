#include "core/LogFileManager.h"

#include <spdlog/spdlog.h>
#include <spdlog/sinks/rotating_file_sink.h>

#include <algorithm>
#include <chrono>

using namespace embview::core;

LogFileManager::LogFileManager(std::filesystem::path logDir)
    : m_logDir(std::move(logDir))
{
}

LogFileManager::~LogFileManager()
{
    if (m_fileSink)
    {
        auto& sinks = spdlog::default_logger()->sinks();
        sinks.erase(std::ranges::remove(sinks, m_fileSink).begin(), sinks.end());
    }
}

void LogFileManager::init()
{
    std::error_code ec;
    std::filesystem::create_directories(m_logDir, ec);
    if (ec)
    {
        spdlog::error("Failed to create log directory '{}': {}", m_logDir.string(), ec.message());
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm{};
#ifdef _WIN32
    localtime_s(&tm, &time);
#else
    localtime_r(&time, &tm);
#endif

    char filename[64];
    std::strftime(filename, sizeof(filename), "embview_%Y%m%d_%H%M%S.log", &tm);
    m_currentLog = m_logDir / filename;

    try
    {
        // 5 MB max, 3 rotated files
        m_fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
            m_currentLog.string(), 5 * 1024 * 1024, 3);

        spdlog::default_logger()->sinks().push_back(m_fileSink);
        spdlog::info("Log file: {}", m_currentLog.string());
    }
    catch (const spdlog::spdlog_ex& e)
    {
        spdlog::error("Failed to create log file sink: {}", e.what());
        m_fileSink.reset();
    }
}

std::filesystem::path LogFileManager::currentLogPath() const
{
    return m_currentLog;
}

std::vector<std::filesystem::path> LogFileManager::listLogs() const
{
    std::vector<std::filesystem::path> result;

    if (!std::filesystem::exists(m_logDir))
    {
        return result;
    }

    std::error_code ec;
    for (const auto& entry : std::filesystem::directory_iterator(m_logDir, ec))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".log")
        {
            result.push_back(entry.path());
        }
    }

    // Sort newest first, using error_code overload to handle deleted files
    std::ranges::sort(result, [](const auto& a, const auto& b)
    {
        std::error_code ecA, ecB;
        auto timeA = std::filesystem::last_write_time(a, ecA);
        auto timeB = std::filesystem::last_write_time(b, ecB);
        if (ecA || ecB) return false;
        return timeA > timeB;
    });

    return result;
}

void LogFileManager::deleteLog(const std::filesystem::path& path) const
{
    if (path == m_currentLog)
    {
        spdlog::warn("Cannot delete the active log file");
        return;
    }

    std::error_code ec;
    std::filesystem::remove(path, ec);
    if (ec)
    {
        spdlog::error("Failed to delete log: {}", ec.message());
    }
}

void LogFileManager::deleteOlderThan(const std::chrono::hours maxAge) const
{
    const auto now = std::filesystem::file_time_type::clock::now();

    for (const auto& logPath : listLogs())
    {
        if (logPath == m_currentLog)
        {
            continue;
        }

        std::error_code ec;
        auto writeTime = std::filesystem::last_write_time(logPath, ec);
        if (ec)
        {
            continue;
        }

        const auto age = std::chrono::duration_cast<std::chrono::hours>(now - writeTime);
        if (age > maxAge)
        {
            deleteLog(logPath);
        }
    }
}
