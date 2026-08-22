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
        /**
         * @brief Construct a ConfigManager with an optional path to the configuration file.
         * @param configPath The path to the configuration
         */
        explicit ConfigManager(std::filesystem::path configPath = "embview_config.json");

        /**
         * @brief Loads the configuration
         * @return True if loaded, false otherwise
         */
        bool load();

        /**
         * @brief Saves the configuration to the file.
         * @return True if saved, false otherwise.
         */
        [[nodiscard]] bool save() const;

        /**
         * @brief Gets the JSON data
         * @return A Reference to the json data
         */
        nlohmann::json& data();

        /**
         * @brief Gets the JSON data (const version)
         * @return A const refernce to the json data
         */
        [[nodiscard]] const nlohmann::json& data() const;

        /**
         * @brief Gets a value from the JSON data
         * @tparam T The type of the value to get
         * @param key The key to look up
         * @param defaultVal The default value to return if the key is not found
         * @return The value associated with the key, or the default value if not found
         */
        template <typename T>
        T get(const std::string& key, const T& defaultVal) const
        {
            if (m_data.contains(key))
            {
                return m_data[key].get<T>();
            }
            return defaultVal;
        }

        /**
         * @brief Sets a value from the jaon data
         * @tparam T The type of the value to set
         * @param key The key to set
         * @param value The value to set
         */
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
