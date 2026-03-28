#pragma once

#include <filesystem>
#include <string>

#include <nlohmann/json.hpp>

namespace embview::core
{
    /**
     * @brief Saves and loads application configuration to/from JSON.
     *
     * Stores connection presets, channel names/colors, window layout,
     * and trigger configurations.
     */
    class ConfigManager
    {
    public:
        explicit ConfigManager(std::filesystem::path configPath = "embview_config.json");

        bool load();

        bool save() const;

        nlohmann::json& data();

        const nlohmann::json& data() const;

        template <typename T>
        T get(const std::string& key, const T& defaultVal) const
        {
            if (m_data.contains(key))
            {
                return m_data[key].get<T>();
            }
            return defaultVal;
        }

        template <typename T>
        void set(const std::string& key, const T& value)
        {
            m_data[key] = value;
        }

    private:
        std::filesystem::path m_configPath;
        nlohmann::json m_data;
    };
} // namespace embview::core
