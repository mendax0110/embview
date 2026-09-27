#pragma once

#include "core/ITransport.h"

#include <cstdint>
#include <queue>
#include <span>
#include <vector>

namespace embview::testing
{
    /**
    * @brief Mock transport for unit testing.
    *
    * Allows feeding canned byte sequences that will be returned by read().
    * Records all bytes passed to write().
    */
    class MockTransport final : public core::ITransport
    {
    public:

        /**
         * @brief
         * @return
         */
        bool open() override
        {
            m_isOpen = true;
            return true;
        }

        /**
         * @brief
         */
        void close() override
        {
            m_isOpen = false;
        }

        /**
         * @brief
         * @return
         */
        bool isOpen() const override
        {
            return m_isOpen;
        }

        /**
         * @brief
         * @param maxBytes
         * @return
         */
        std::vector<uint8_t> read(const std::size_t maxBytes) override
        {
            if (m_readData.empty())
            {
                return {};
            }

            auto& front = m_readData.front();
            const std::size_t count = std::min(maxBytes, front.size());
            std::vector<uint8_t> result(front.begin(), front.begin() + count);
            front.erase(front.begin(), front.begin() + count);

            if (front.empty())
            {
                m_readData.pop();
            }

            return result;
        }

        /**
         * @brief
         * @param data
         * @return
         */
        std::size_t write(std::span<const uint8_t> data) override
        {
            m_writtenData.insert(m_writtenData.end(), data.begin(), data.end());
            return data.size();
        }

        /**
         * @brief Enqueue data to be returned by subsequent read() calls.
         * @param data
         */
        void enqueueReadData(std::vector<uint8_t> data)
        {
            m_readData.push(std::move(data));
        }

        /**
         * @brief Get all data written via write().
         * @return
         */
        const std::vector<uint8_t>& getWrittenData() const
        {
            return m_writtenData;
        }

        /**
         * @brief Clear recorded write data.
         */
        void clearWrittenData()
        {
            m_writtenData.clear();
        }

    private:
        bool m_isOpen = false;
        std::queue<std::vector<uint8_t>> m_readData;
        std::vector<uint8_t> m_writtenData;
    };
} // namespace embview::testing
