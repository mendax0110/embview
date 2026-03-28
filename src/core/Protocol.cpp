#include "core/Protocol.h"

#include <array>
#include <cstring>

using namespace embview::core;

void Protocol::feedData(std::span<const uint8_t> data)
{
    m_buffer.insert(m_buffer.end(), data.begin(), data.end());
}

std::optional<DataFrame> Protocol::parseNext()
{
    while (m_buffer.size() >= FRAME_SIZE)
    {
        // Scan for start byte
        if (m_buffer[0] != START_BYTE)
        {
            m_buffer.erase(m_buffer.begin());
            continue;
        }

        uint8_t length = m_buffer[1];
        // Length should be CHANNEL(1) + PAYLOAD(8) = 9
        if (length != 1 + PAYLOAD_SIZE)
        {
            m_buffer.erase(m_buffer.begin());
            continue;
        }

        // Verify CRC over bytes [1..FRAME_SIZE-2] (LENGTH + CHANNEL + PAYLOAD)
        std::span<const uint8_t> crcRegion(m_buffer.data() + 1, FRAME_SIZE - 2);
        uint8_t expectedCrc = crc8(crcRegion);
        uint8_t receivedCrc = m_buffer[FRAME_SIZE - 1];

        if (expectedCrc != receivedCrc)
        {
            m_buffer.erase(m_buffer.begin());
            continue;
        }

        DataFrame frame;
        frame.channel = m_buffer[2];
        frame.timestamp = m_timestampCounter;
        m_timestampCounter += 1.0;

        std::memcpy(&frame.value, m_buffer.data() + 3, sizeof(double));

        m_buffer.erase(m_buffer.begin(), m_buffer.begin() + FRAME_SIZE);
        return frame;
    }

    return std::nullopt;
}

uint8_t Protocol::crc8(std::span<const uint8_t> data)
{
    uint8_t crc = 0x00;
    for (uint8_t byte : data)
    {
        crc ^= byte;
        for (int i = 0; i < 8; ++i)
        {
            if (crc & 0x80)
            {
                crc = static_cast<uint8_t>((crc << 1) ^ 0x07);
            }
            else
            {
                crc = static_cast<uint8_t>(crc << 1);
            }
        }
    }
    return crc;
}

std::vector<uint8_t> Protocol::encode(const DataFrame& frame)
{
    std::vector<uint8_t> result;
    result.reserve(FRAME_SIZE);

    result.push_back(START_BYTE);

    uint8_t length = static_cast<uint8_t>(1 + PAYLOAD_SIZE);
    result.push_back(length);
    result.push_back(static_cast<uint8_t>(frame.channel));

    std::array<uint8_t, sizeof(double)> valueBytes{};
    std::memcpy(valueBytes.data(), &frame.value, sizeof(double));
    result.insert(result.end(), valueBytes.begin(), valueBytes.end());

    // CRC over [LENGTH, CHANNEL, PAYLOAD]
    std::span<const uint8_t> crcRegion(result.data() + 1, result.size() - 1);
    result.push_back(crc8(crcRegion));

    return result;
}
