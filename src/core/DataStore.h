#pragma once

#include "core/Protocol.h"

#include <cstdint>
#include <deque>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

/// @brief Namespace for core data management and protocol parsing. \namespace embview::core
namespace embview::core
{
    /**
    * @brief Thread-safe storage for incoming data frames.
    *
    * Maintains per-channel ring buffers with configurable maximum capacity.
    * Supports concurrent reads from the UI thread and writes from the I/O thread.
    */
    class DataStore
    {
    public:
        /**
         * @brief Construct a DataStore with the given per-channel capacity.
         * @param maxSamplesPerChannel Maximum number of frames per channel before oldest are dropped.
         */
        explicit DataStore(std::size_t maxSamplesPerChannel = 10000);
        ~DataStore();

        /**
         * @brief Push a new data frame into the appropriate channel buffer.
         * @param frame The parsed data frame to store.
         */
        void push(const DataFrame& frame);

        /**
         * @brief Retrieve all stored frames for a channel.
         * @param channel The channel identifier to query.
         * @return Vector of stored frames for the channel.
         */
        std::vector<DataFrame> getChannel(uint16_t channel) const;

        std::vector<uint16_t> getActiveChannels() const;

        std::size_t getChannelSize(uint16_t channel) const;

        void clear();

        void clearChannel(uint16_t channel);

    private:
        std::size_t m_maxSamples;
        mutable std::shared_mutex m_mutex;
        std::unordered_map<uint16_t, std::deque<DataFrame>> m_channels;
    };
} // namespace embview::core
