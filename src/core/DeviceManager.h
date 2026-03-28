#pragma once

#include "core/IProtocolParser.h"
#include "core/ITransport.h"
#include "core/Protocol.h"

#include <cstdint>
#include <memory>
#include <shared_mutex>
#include <span>
#include <string>
#include <thread>
#include <vector>

namespace embview::core
{
    class DataStore;
    class RawDataBuffer;

    /**
     * @brief Manages multiple concurrent device connections.
     *
     * Each device runs its own reader thread feeding parsed frames into a shared
     * DataStore. Channel IDs are remapped as deviceIndex * 256 + frame.channel
     * to avoid collisions between devices.
     */
    class DeviceManager
    {
    public:
        explicit DeviceManager(std::shared_ptr<DataStore> dataStore);
        ~DeviceManager();

        DeviceManager(const DeviceManager&) = delete;
        DeviceManager& operator=(const DeviceManager&) = delete;

        /// @brief Add a new device connection and start its reader thread.
        void addDevice(std::string name, std::shared_ptr<ITransport> transport,
                       ProtocolMode protocolMode = ProtocolMode::Binary);

        /// @brief Disconnect and remove a device by name.
        void removeDevice(const std::string& name);

        /// @brief Disconnect and remove all devices.
        void removeAll();

        /// @brief Get the list of connected device names.
        std::vector<std::string> getDeviceNames() const;

        /// @brief Check if a device is currently connected and its transport is open.
        bool isDeviceConnected(const std::string& name) const;

        /// @brief Get the number of connected devices.
        std::size_t deviceCount() const;

        /// @brief Send a command frame to a specific device (binary protocol encoding).
        void sendCommand(const std::string& deviceName, const DataFrame& frame);

        /// @brief Send raw bytes to a specific device (no protocol encoding).
        void sendRaw(const std::string& deviceName, std::span<const uint8_t> data);

        /// @brief Get the shared raw data buffer for hex view.
        std::shared_ptr<RawDataBuffer> getRawDataBuffer() const;

    private:
        struct DeviceSession
        {
            std::string name;
            uint8_t deviceIndex = 0;
            std::shared_ptr<ITransport> transport;
            std::unique_ptr<IProtocolParser> parser;
            std::jthread readerThread;
        };

        void readerLoop(DeviceSession& session, std::stop_token stopToken);
        uint8_t nextDeviceIndex();

        std::shared_ptr<DataStore> m_dataStore;
        std::shared_ptr<RawDataBuffer> m_rawDataBuffer;
        mutable std::shared_mutex m_mutex;
        std::vector<std::unique_ptr<DeviceSession>> m_sessions;
        uint8_t m_nextIndex = 0;
    };
} // namespace embview::core
