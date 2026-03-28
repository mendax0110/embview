#pragma once

#include "core/ITransport.h"

#include <cstdint>
#include <memory>
#include <string>

struct sp_port;

namespace embview::core
{
    /// @brief Serial port configuration parameters.
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
        ~SerialTransport() override;

        SerialTransport(const SerialTransport&) = delete;
        SerialTransport& operator=(const SerialTransport&) = delete;
        SerialTransport(SerialTransport&&) noexcept;
        SerialTransport& operator=(SerialTransport&&) noexcept;

        bool open() override;
        void close() override;
        bool isOpen() const override;
        std::vector<uint8_t> read(std::size_t maxBytes) override;
        std::size_t write(std::span<const uint8_t> data) override;

        /**
        * @brief Enumerate available serial ports on the system.
        * @return List of port names (e.g. "COM3", "/dev/ttyUSB0").
        */
        static std::vector<std::string> listPorts();

    private:
        struct PortDeleter
        {
            void operator()(sp_port* port) const;
        };

        SerialConfig m_config;
        std::unique_ptr<sp_port, PortDeleter> m_port;
        bool m_isOpen = false;
    };
} // namespace embview::core
