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

        /**
         * @brief Add a new device connection and start its reader thread.
         * @param name The name of the device to add.
         * @param transport The transport to use for the device.
         * @param protocolMode The protocol mode to use for the device.
         */
        void addDevice(std::string name, std::shared_ptr<ITransport> transport,
                       ProtocolMode protocolMode = ProtocolMode::Binary);

        /**
         * @brief Disconnect and remove a device by name.
         * @param name The name of the device to remove.
         */
        void removeDevice(const std::string& name);

        /**
         * @brief Disconnect and remove all devices.
         */
        void removeAll();

        /**
         * @brief Get the list of connected device names.
         * @return A vector containing the names of all connected devices.
         */
        std::vector<std::string> getDeviceNames() const;

        /**
         * @brief Check if a device is currently connected and its transport is open.
         * @param name The name of the device to check.
         * @return true if the device is connected and its transport is open, false otherwise.
         */
        bool isDeviceConnected(const std::string& name) const;

        /**
         * @brief Get the number of connected devices.
         * @return The number of connected devices.
         */
        std::size_t deviceCount() const;

        /**
         * @brief Send a command frame to a specific device (binary protocol encoding).
         * @param deviceName The name of the device to send the command to.
         * @param frame The command frame to send.
         */
        void sendCommand(const std::string& deviceName, const DataFrame& frame) const;

        /**
         * @brief Send raw bytes to a specific device (no protocol encoding).
         * @param deviceName The name of the device to send the raw bytes to.
         * @param data The raw bytes to send.
         */
        void sendRaw(const std::string& deviceName, std::span<const uint8_t> data) const;

        /**
         * @brief Get the shared raw data buffer for hex view.
         * @return A vector containing the raw buffer data
         */
        std::shared_ptr<RawDataBuffer> getRawDataBuffer() const;

    private:

        /**
         * @brief Struct representing the Device Session \struct DeviceSession
         */
        struct DeviceSession
        {
            std::string name;
            uint8_t deviceIndex = 0;
            std::shared_ptr<ITransport> transport;
            std::unique_ptr<IProtocolParser> parser;
            std::jthread readerThread;
        };

        /**
         * @brief The main loop for reading data from a device.
         * @param session The device session
         * @param stopToken The stop token
         */
        void readerLoop(DeviceSession& session, const std::stop_token& stopToken) const;

        /**
         * @brief Getter for the next device index
         * @return The next device index
         */
        uint8_t nextDeviceIndex();

        std::shared_ptr<DataStore> m_dataStore;
        std::shared_ptr<RawDataBuffer> m_rawDataBuffer;
        mutable std::shared_mutex m_mutex;
        std::vector<std::unique_ptr<DeviceSession>> m_sessions;
        uint8_t m_nextIndex = 0;
    };
} // namespace embview::core
