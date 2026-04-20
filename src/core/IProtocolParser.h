#pragma once

#include "core/Protocol.h"

#include <cstdint>
#include <optional>
#include <span>
#include <string>

namespace embview::core
{
    /// @brief Protocol parser mode identifiers.
    enum class ProtocolMode
    {
        Binary,    ///< Custom binary frame [0xAA][LEN][CH][double][CRC8]
        AsciiLine, ///< ASCII line-based: "value\n" or "channel:value\n"
        AsciiCsv   ///< ASCII CSV: "ch0,ch1,ch2,...\n"
    };

    /**
     * @brief Abstract protocol parser interface.
     *
     * Allows the DeviceManager to work with different wire formats.
     * Feed raw bytes, then poll for parsed frames.
     */
    class IProtocolParser
    {
    public:
        virtual ~IProtocolParser() = default;

        virtual void feedData(std::span<const uint8_t> data) = 0;

        virtual std::optional<DataFrame> parseNext() = 0;

        [[nodiscard]] virtual ProtocolMode mode() const = 0;
    };

    /**
     * @brief Adapter wrapping the existing binary Protocol as an IProtocolParser.
     */
    class BinaryProtocolParser : public IProtocolParser
    {
    public:
        void feedData(const std::span<const uint8_t> data) override
        {
            m_protocol.feedData(data);
        }

        std::optional<DataFrame> parseNext() override
        {
            return m_protocol.parseNext();
        }

        [[nodiscard]] ProtocolMode mode() const override { return ProtocolMode::Binary; }

    private:
        Protocol m_protocol;
    };

    /**
     * @brief ASCII line-based protocol parser.
     *
     * Accepts newline-terminated text. Supports formats:
     *   - "value"           -> channel 0, parsed double
     *   - "channel:value"   -> specific channel, parsed double
     *   - "name:value"      -> channel 0, parsed double (name ignored for now)
     */
    class AsciiLineParser : public IProtocolParser
    {
    public:
        void feedData(std::span<const uint8_t> data) override;

        std::optional<DataFrame> parseNext() override;

        [[nodiscard]] ProtocolMode mode() const override { return ProtocolMode::AsciiLine; }

    private:
        std::string m_buffer;
        double m_timestampCounter = 0.0;
    };

    /**
     * @brief ASCII CSV protocol parser.
     *
     * Each line is comma-separated values, one per channel:
     *   "1.23,4.56,7.89\n" -> ch0=1.23, ch1=4.56, ch2=7.89
     */
    class AsciiCsvParser : public IProtocolParser
    {
    public:
        void feedData(std::span<const uint8_t> data) override;

        std::optional<DataFrame> parseNext() override;

        [[nodiscard]] ProtocolMode mode() const override { return ProtocolMode::AsciiCsv; }

    private:
        std::string m_buffer;
        std::vector<DataFrame> m_pending;
        double m_timestampCounter = 0.0;
    };
} // namespace embview::core
