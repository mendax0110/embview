#include "core/ConfigManager.h"

#include <spdlog/spdlog.h>

#include <fstream>

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
        std::ifstream file(m_configPath);
        if (!file.is_open())
        {
            spdlog::error("Failed to open config file: {}", m_configPath.string());
            m_data = nlohmann::json::object();
            return false;
        }

        m_data = nlohmann::json::parse(file);
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
        std::ofstream file(m_configPath);
        if (!file.is_open())
        {
            spdlog::error("Failed to write config file: {}", m_configPath.string());
            return false;
        }

        file << m_data.dump(4);

        if (!file.good())
        {
            spdlog::error("Write error saving config to {}", m_configPath.string());
            return false;
        }

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
