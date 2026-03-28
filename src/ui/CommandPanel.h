#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace embview::core
{
    class DeviceManager;
} // namespace embview::core

namespace embview::ui
{
    /**
     * @brief Panel for building and sending commands to connected devices.
     *
     * Supports three modes:
     * - Raw text: sends arbitrary strings with configurable line endings.
     * - Hex bytes: sends raw hex-encoded byte sequences (e.g. "AA 0B FF").
     * - Binary frame: encodes a channel+value pair using the wire protocol.
     *
     * Maintains a scrollable history of sent commands.
     */
    class CommandPanel
    {
    public:
        explicit CommandPanel(std::shared_ptr<core::DeviceManager> deviceMgr);
        ~CommandPanel();

        void render(bool& open);

    private:
        static std::vector<uint8_t> parseHexString(const std::string& hex);

        struct HistoryEntry
        {
            std::string device;
            std::string text;
        };

        std::shared_ptr<core::DeviceManager> m_deviceMgr;
        int m_selectedDevice = 0;

        // Send mode: 0=Raw Text, 1=Hex Bytes, 2=Binary Frame
        int m_sendMode = 0;

        // Raw text / hex input
        char m_textInput[512] = {};

        // Line ending: 0=\r\n, 1=\n, 2=\r, 3=None
        int m_lineEnding = 0;

        // Binary frame fields
        int m_channel = 0;
        double m_value = 0.0;

        std::vector<HistoryEntry> m_history;
    };
} // namespace embview::ui
