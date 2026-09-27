#pragma once

#include "core/ITransport.h"

#include <cstdint>
#include <memory>
#include <string>

/**
 * @brief Sp port.
 * \struct sp_port
 */
struct sp_port;

/**
 * @namespace embview::core Core support.
 */
namespace embview::core
{
    /**
     * @brief Serial port configuration parameters. \struct SerialConfig
     */
    struct SerialConfig
    {
        std::string portName;
        int baudRate = 115200;
        int dataBits = 8;
        int stopBits = 1;
        int parity   = 0; ///< 0 = none, 1 = odd, 2 = even
    };

    /**
     * @brief USB/Serial transport using libserialport.
     *
     * Wraps a native serial port behind the ITransport interface.
     * Owns the port handle via custom deleter.
     */
    class SerialTransport final : public ITransport
    {
    public:
        /**
         * @brief Construct a serial transport with the given configuration.
         * @param config Serial port parameters (port name, baud rate, etc.).
         */
        explicit SerialTransport(SerialConfig config);

        /**
         * @brief Destroy the serial transport and close the port.
         */
        ~SerialTransport() override;

        /**
         * @brief Serial transport is not copyable.
         */
        SerialTransport(const SerialTransport&) = delete;

        /**
         * @brief Serial transport is not copy-assignable.
         */
        SerialTransport& operator=(const SerialTransport&) = delete;
        /**
          * @brief Move-construct a serial transport.
          * @param other Source transport to move from.
          */
        SerialTransport(SerialTransport&& other) noexcept;

        /**
          * @brief Move-assign a serial transport.
          * @param other Source transport to move from.
          * @return Reference to this transport.
          */
        SerialTransport& operator=(SerialTransport&& other) noexcept;

        /**
         * @brief Open the configured serial port.
         * @return true on success.
         */
        bool open() override;

        /**
         * @brief Close the serial port.
         */
        void close() override;

        /**
         * @brief Check whether the serial port is open.
         * @return true when the port is connected.
         */
        [[nodiscard]] bool isOpen() const override;

        /**
         * @brief Read up to @p maxBytes bytes from the serial port.
         * @param maxBytes Maximum number of bytes to read.
         * @return Bytes received from the port.
         */
        std::vector<uint8_t> read(std::size_t maxBytes) override;

        /**
         * @brief Write data to the serial port.
         * @param data Bytes to transmit.
         * @return Number of bytes written.
         */
        std::size_t write(std::span<const uint8_t> data) override;

        /**
        * @brief Enumerate available serial ports on the system.
        * @return List of port names (e.g. "COM3", "/dev/ttyUSB0").
        */
        static std::vector<std::string> listPorts();

    private:
        /**
         * @brief Port Deleter.
         * \struct PortDeleter
         */
        struct PortDeleter
        {
            /**
            * @brief Release a native serial port handle.
            * @param port Native libserialport handle to close.
            */
            void operator()(sp_port* port) const;
        };

        SerialConfig m_config;
        std::unique_ptr<sp_port, PortDeleter> m_port;
        bool m_isOpen = false;
    };
} // namespace embview::core
