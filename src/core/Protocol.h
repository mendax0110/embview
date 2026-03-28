#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <vector>

namespace embview::core
{
    /// @brief A single parsed data frame from a device.
    struct DataFrame
    {
        uint16_t channel;
        double  timestamp;
        double  value;
    };

    /**
    * @brief Frame protocol parser for device communication.
    *
    * Wire format: [0xAA][LENGTH][CHANNEL][PAYLOAD (8 bytes: double)][CRC8]
    * - START_BYTE: 0xAA
    * - LENGTH: payload size + 1 (channel byte)
    * - CHANNEL: data channel identifier
    * - PAYLOAD: 8-byte little-endian double value
    * - CRC8: CRC over LENGTH+CHANNEL+PAYLOAD bytes
    *
    * Feed raw bytes via feedData(), then poll parseNext() for frames.
    */
    class Protocol
    {
    public:
        static constexpr uint8_t START_BYTE  = 0xAA;
        static constexpr std::size_t HEADER_SIZE = 2;   ///< START + LENGTH
        static constexpr std::size_t PAYLOAD_SIZE = 8;   ///< sizeof(double)
        static constexpr std::size_t FRAME_SIZE = HEADER_SIZE + 1 + PAYLOAD_SIZE + 1; ///< full frame

        /**
         * @brief Append raw bytes to the internal buffer.
         * @param data Raw bytes received from the transport.
         */
        void feedData(std::span<const uint8_t> data);

        /**
        * @brief Try to parse the next complete frame from the buffer.
        * @return Parsed DataFrame if a valid frame was found.
        */
        std::optional<DataFrame> parseNext();

        /**
         * @brief Compute CRC8 over a byte range.
         * @param data Bytes to compute the checksum over.
         * @return Computed CRC8 value.
         */
        static uint8_t crc8(std::span<const uint8_t> data);

        /**
         * @brief Encode a DataFrame into wire format.
         * @param frame The data frame to encode.
         * @return Encoded byte sequence ready for transmission.
         */
        static std::vector<uint8_t> encode(const DataFrame& frame);

    private:
        std::vector<uint8_t> m_buffer;
        double m_timestampCounter = 0.0;
    };
} // namespace embview::core
