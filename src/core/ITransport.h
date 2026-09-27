#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

/**
 * @namespace embview::core Core support.
 */
namespace embview::core
{
    /**
     * @brief Abstract transport interface for device communication.
     *
     * All communication backends (serial, TCP, SPI, I2C) implement this
     * interface so the protocol layer remains transport-agnostic.
     */
    class ITransport
    {
    public:
        /**
         * @brief Destroy the transport.
         */
        virtual ~ITransport() = default;

        /**
         * @brief Open the transport connection.
         * @return true on success.
         */
        virtual bool open() = 0;

        /**
         * @brief Close the transport connection.
         */
        virtual void close() = 0;

        /**
         * @brief Check whether the transport is open.
         * @return true if the connection is active.
         */
        [[nodiscard]] virtual bool isOpen() const = 0;

        /**
         * @brief Read up to @p maxBytes bytes.
         * @param maxBytes Maximum number of bytes to read.
         * @return The bytes received.
         */
        virtual std::vector<uint8_t> read(std::size_t maxBytes) = 0;

        /**
         * @brief Write bytes to the transport.
         * @param data Bytes to send.
         * @return Number of bytes written.
         */
        virtual std::size_t write(std::span<const uint8_t> data) = 0;
    };
} // namespace embview::core
