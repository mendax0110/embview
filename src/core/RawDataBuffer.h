#pragma once

#include <cstdint>
#include <mutex>
#include <vector>

/**
 * @namespace embview::core Core support.
 */
namespace embview::core
{
    /**
     * @brief Thread-safe circular buffer capturing raw bytes from transports.
     *
     * Used by the hex view panel to display raw incoming data.
     */
    class RawDataBuffer
    {
    public:
        /**
         * @brief Create a raw byte buffer with the given capacity.
         * @param capacity Maximum number of stored bytes.
         */
        explicit RawDataBuffer(std::size_t capacity = 65536);

        /**
         * @brief Append new raw bytes.
         * @param data Incoming bytes to append.
         */
        void push(const std::vector<uint8_t>& data);

        /**
         * @brief Copy the current buffer contents.
         * @return The current byte snapshot.
         */
        std::vector<uint8_t> snapshot() const;

        /**
         * @brief Clear the stored bytes.
         */
        void clear();

        /**
         * @brief Get the number of stored bytes.
         * @return Current buffer size.
         */
        std::size_t size() const;

    private:
        mutable std::mutex m_mutex;
        std::vector<uint8_t> m_buffer;
        std::size_t m_capacity;
    };
} // namespace embview::core
