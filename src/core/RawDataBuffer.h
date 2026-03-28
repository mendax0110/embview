#pragma once

#include <cstdint>
#include <mutex>
#include <vector>

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
        explicit RawDataBuffer(std::size_t capacity = 65536);

        void push(const std::vector<uint8_t>& data);

        std::vector<uint8_t> snapshot() const;

        void clear();

        std::size_t size() const;

    private:
        mutable std::mutex m_mutex;
        std::vector<uint8_t> m_buffer;
        std::size_t m_capacity;
    };
} // namespace embview::core
