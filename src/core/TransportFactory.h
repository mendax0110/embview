#pragma once

#include "core/ITransport.h"

#include <functional>
#include <memory>
#include <string>
#include <unordered_map>

#include <nlohmann/json_fwd.hpp>

namespace embview::core
{
    /**
    * @brief Factory for creating transport instances by type name.
    *
    * New transport types can be registered at runtime via registerCreator().
    * Built-in types ("serial") are registered automatically.
    */
    class TransportFactory
    {
    public:
        using Creator = std::function<std::unique_ptr<ITransport>(const nlohmann::json&)>;

        /**
         * @brief Get the singleton factory instance.
         * @return Reference to the global TransportFactory.
         */
        static TransportFactory& instance();

        /**
         * @brief Register a transport creator under the given type name.
         * @param type Unique type identifier (e.g. "serial", "tcp").
         * @param creator Factory function that produces an ITransport from JSON config.
         */
        void registerCreator(const std::string& type, Creator creator);

        /**
        * @brief Create a transport of the given type with the provided config.
        * @param type Transport type name (e.g. "serial").
        * @param config JSON configuration for the transport.
        * @return Owning pointer to the created transport.
        * @throws std::runtime_error if type is not registered.
        */
        std::unique_ptr<ITransport> create(const std::string& type, const nlohmann::json& config) const;

        /**
         * @brief Check if a transport type is registered.
         * @param type The type name to look up.
         * @return true if the type has a registered creator.
         */
        bool hasType(const std::string& type) const;

    private:
        TransportFactory();
        std::unordered_map<std::string, Creator> m_creators;
    };
} // namespace embview::core
