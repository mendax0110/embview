#include "core/IProtocolParser.h"

#include <spdlog/spdlog.h>

#include <algorithm>
#include <charconv>
#include <cctype>

using namespace embview::core;

// --- Helpers ---

/// @brief Extracts a line from the buffer, handling \n, \r\n, and \r-only endings.
/// Returns true if a line was extracted, false if no complete line is available.
static bool extractLine(std::string& buffer, std::string& line)
{
    // Find the first \r or \n
    auto pos = buffer.find_first_of("\r\n");
    if (pos == std::string::npos)
    {
        // No line terminator yet. But if the buffer is very large, treat it
        // as a broken stream and flush it as a line to avoid unbounded growth.
        if (buffer.size() > 4096)
        {
            line = std::move(buffer);
            buffer.clear();
            return true;
        }
        return false;
    }

    line = buffer.substr(0, pos);

    // Consume \r\n, \n, or \r
    if (pos < buffer.size() && buffer[pos] == '\r')
    {
        ++pos;
        if (pos < buffer.size() && buffer[pos] == '\n')
        {
            ++pos;
        }
    }
    else if (buffer[pos] == '\n')
    {
        ++pos;
    }

    buffer.erase(0, pos);
    return true;
}

/// @brief Trims leading whitespace from a string_view.
static std::string_view trimLeft(std::string_view sv)
{
    while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.front())))
    {
        sv.remove_prefix(1);
    }
    return sv;
}

/// @brief Trims trailing whitespace from a string_view.
static std::string_view trimRight(std::string_view sv)
{
    while (!sv.empty() && std::isspace(static_cast<unsigned char>(sv.back())))
    {
        sv.remove_suffix(1);
    }
    return sv;
}

/// @brief Trims both ends.
static std::string_view trim(std::string_view sv)
{
    return trimRight(trimLeft(sv));
}

/// @brief Tries to parse a double from the string, skipping any leading non-numeric characters.
/// Returns true if a number was found, sets outValue and advances 'sv' past the number.
static bool scanDouble(std::string_view sv, double& outValue, std::string_view& remainder)
{
    sv = trimLeft(sv);

    // Skip to the first character that could start a number: digit, '.', '+', '-'
    std::size_t startIdx = 0;
    while (startIdx < sv.size())
    {
        char c = sv[startIdx];
        if (std::isdigit(static_cast<unsigned char>(c)) || c == '.' || c == '-' || c == '+')
        {
            break;
        }
        ++startIdx;
    }

    if (startIdx >= sv.size())
    {
        return false;
    }

    std::string_view numView = sv.substr(startIdx);
    auto [ptr, ec] = std::from_chars(numView.data(), numView.data() + numView.size(), outValue);
    if (ec != std::errc())
    {
        return false;
    }

    remainder = std::string_view(ptr, numView.data() + numView.size() - ptr);
    return true;
}

// --- AsciiLineParser ---

void AsciiLineParser::feedData(std::span<const uint8_t> data)
{
    m_buffer.append(reinterpret_cast<const char*>(data.data()), data.size());
}

std::optional<DataFrame> AsciiLineParser::parseNext()
{
    while (true)
    {
        std::string line;
        if (!extractLine(m_buffer, line))
        {
            return std::nullopt;
        }

        std::string_view trimmedLine = trim(line);
        if (trimmedLine.empty())
        {
            continue;
        }

        DataFrame frame{};
        frame.timestamp = m_timestampCounter;
        m_timestampCounter += 1.0;

        // Try "key:value" or "channel:value" format (split on first ':')
        auto colonPos = trimmedLine.find(':');
        if (colonPos != std::string_view::npos)
        {
            std::string_view keyPart = trim(trimmedLine.substr(0, colonPos));
            std::string_view valuePart = trim(trimmedLine.substr(colonPos + 1));

            // Try parsing key as channel number
            uint16_t ch = 0;
            auto [ptr, ec] = std::from_chars(keyPart.data(), keyPart.data() + keyPart.size(), ch);
            if (ec != std::errc())
            {
                ch = 0; // Non-numeric key (e.g., "temp"), default to channel 0
            }
            frame.channel = ch;

            // Parse value -- scan for first number in the value part
            double val = 0.0;
            std::string_view remainder;
            if (!scanDouble(valuePart, val, remainder))
            {
                spdlog::debug("AsciiLineParser: skipping unparseable line: {}", line);
                continue;
            }
            frame.value = val;
        }
        else
        {
            // No colon -- try "key=value" format
            auto eqPos = trimmedLine.find('=');
            if (eqPos != std::string_view::npos)
            {
                std::string_view keyPart = trim(trimmedLine.substr(0, eqPos));
                std::string_view valuePart = trim(trimmedLine.substr(eqPos + 1));

                uint16_t ch = 0;
                auto [ptr, ec] = std::from_chars(keyPart.data(), keyPart.data() + keyPart.size(), ch);
                if (ec != std::errc())
                {
                    ch = 0;
                }
                frame.channel = ch;

                double val = 0.0;
                std::string_view remainder;
                if (!scanDouble(valuePart, val, remainder))
                {
                    spdlog::debug("AsciiLineParser: skipping unparseable line: {}", line);
                    continue;
                }
                frame.value = val;
            }
            else
            {
                // Plain value format -- scan for first number anywhere in the line
                frame.channel = 0;
                double val = 0.0;
                std::string_view remainder;
                if (!scanDouble(trimmedLine, val, remainder))
                {
                    spdlog::debug("AsciiLineParser: skipping unparseable line: {}", line);
                    continue;
                }
                frame.value = val;
            }
        }

        return frame;
    }
}

// --- AsciiCsvParser ---

void AsciiCsvParser::feedData(std::span<const uint8_t> data)
{
    m_buffer.append(reinterpret_cast<const char*>(data.data()), data.size());
}

std::optional<DataFrame> AsciiCsvParser::parseNext()
{
    // Return buffered frames first
    if (!m_pending.empty())
    {
        auto frame = m_pending.front();
        m_pending.erase(m_pending.begin());
        return frame;
    }

    while (true)
    {
        std::string line;
        if (!extractLine(m_buffer, line))
        {
            return std::nullopt;
        }

        std::string_view trimmedLine = trim(line);
        if (trimmedLine.empty())
        {
            continue;
        }

        double ts = m_timestampCounter;
        m_timestampCounter += 1.0;

        // Parse comma-separated or semicolon/tab-separated values
        uint16_t ch = 0;
        std::size_t start = 0;
        std::string lineStr(trimmedLine);
        while (start < lineStr.size())
        {
            // Find next separator (comma, semicolon, or tab)
            auto sepPos = lineStr.find_first_of(",;\t", start);
            std::size_t end = (sepPos == std::string::npos) ? lineStr.size() : sepPos;

            std::string_view token = trim(std::string_view(lineStr).substr(start, end - start));

            double val = 0.0;
            std::string_view remainder;
            if (scanDouble(token, val, remainder))
            {
                DataFrame frame{};
                frame.channel = ch;
                frame.timestamp = ts;
                frame.value = val;
                m_pending.push_back(frame);
            }

            ++ch;
            start = end + 1;
        }

        if (!m_pending.empty())
        {
            auto frame = m_pending.front();
            m_pending.erase(m_pending.begin());
            return frame;
        }

        spdlog::debug("AsciiCsvParser: skipping unparseable line: {}", line);
    }
}
