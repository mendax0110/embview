#include "core/ConfigManager.h"
#include "core/FileFactory.h"
#include <spdlog/spdlog.h>
#include <vector>

using namespace embview::core;

ConfigManager::ConfigManager(std::filesystem::path configPath)
    : m_configPath(std::move(configPath))
{
}

bool ConfigManager::load()
{
    std::error_code ec;
    if (!std::filesystem::exists(m_configPath, ec))
    {
        spdlog::info("No config file found at {}, using defaults", m_configPath.string());
        m_data = nlohmann::json::object();
        return false;
    }

    try
    {
        const auto blob = FileFactory::instance().loadFromFile(m_configPath, FileTypeId::json);
        const std::string jsonText(blob->data().begin(), blob->data().end());
        m_data = nlohmann::json::parse(jsonText);
        spdlog::info("Loaded config from {}", m_configPath.string());
        return true;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Failed to parse config file: {}", e.what());
        m_data = nlohmann::json::object();
        return false;
    }
}

bool ConfigManager::save() const
{
    try
    {
        const auto payload = m_data.dump(4);
        const std::vector<uint8_t> bytes(payload.begin(), payload.end());

        const auto blob = FileFactory::instance().create(FileTypeId::json, bytes);
        FileFactory::saveToFile(*blob, m_configPath);

        spdlog::info("Saved config to {}", m_configPath.string());
        return true;
    }
    catch (const std::exception& e)
    {
        spdlog::error("Failed to save config: {}", e.what());
        return false;
    }
}

nlohmann::json& ConfigManager::data()
{
    return m_data;
}

const nlohmann::json& ConfigManager::data() const
{
    return m_data;
}
