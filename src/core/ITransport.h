#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

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
        virtual ~ITransport() = default;

        /**
        * @brief Open the transport connection.
        * @return true on success, false on failure.
        */
        virtual bool open() = 0;

        /// @brief Close the transport connection.
        virtual void close() = 0;

        /**
         * @brief Check whether the transport is currently open.
         * @return true if the connection is active.
         */
        virtual bool isOpen() const = 0;

        /**
        * @brief Read up to @p maxBytes from the transport.
        * @param maxBytes Maximum number of bytes to read.
        * @return Bytes read (may be fewer than @p maxBytes).
        */
        virtual std::vector<uint8_t> read(std::size_t maxBytes) = 0;

        /**
        * @brief Write data to the transport.
        * @param data Bytes to write.
        * @return Number of bytes actually written.
        */
        virtual std::size_t write(std::span<const uint8_t> data) = 0;
    };
} // namespace embview::core
