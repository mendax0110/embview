#include "ui/CommandPanel.h"

#include "core/DeviceManager.h"
#include "core/Protocol.h"

#include <imgui.h>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <charconv>
#include <format>
#include <ranges>

using namespace embview::ui;
using namespace embview::core;

CommandPanel::CommandPanel(std::shared_ptr<DeviceManager> deviceMgr)
    : m_deviceMgr(std::move(deviceMgr))
{
}

CommandPanel::~CommandPanel() = default;

std::vector<uint8_t> CommandPanel::parseHexString(const std::string& hex)
{
    std::vector<uint8_t> bytes;
    std::size_t i = 0;

    while (i < hex.size())
    {
        // Skip whitespace and common separators
        if (std::isspace(static_cast<unsigned char>(hex[i])) || hex[i] == ',' || hex[i] == ':')
        {
            ++i;
            continue;
        }

        // Skip optional "0x" prefix
        if (i + 1 < hex.size() && hex[i] == '0' && (hex[i + 1] == 'x' || hex[i + 1] == 'X'))
        {
            i += 2;
            continue;
        }

        // Need at least one hex digit
        if (!std::isxdigit(static_cast<unsigned char>(hex[i])))
        {
            ++i;
            continue;
        }

        // Parse up to two hex digits
        std::size_t end = i + 1;
        if (end < hex.size() && std::isxdigit(static_cast<unsigned char>(hex[end])))
        {
            ++end;
        }

        uint8_t val = 0;
        std::from_chars(hex.data() + i, hex.data() + end, val, 16);
        bytes.push_back(val);
        i = end;
    }

    return bytes;
}

void CommandPanel::render(bool& open)
{
    if (!ImGui::Begin("Command", &open))
    {
        ImGui::End();
        return;
    }

    const auto deviceNames = m_deviceMgr->getDeviceNames();

    if (deviceNames.empty())
    {
        ImGui::TextDisabled("No devices connected");
        ImGui::End();
        return;
    }

    // Device selector
    if (m_selectedDevice >= static_cast<int>(deviceNames.size()))
    {
        m_selectedDevice = 0;
    }

    if (ImGui::BeginCombo("Target Device", deviceNames[m_selectedDevice].c_str()))
    {
        for (int i = 0; i < static_cast<int>(deviceNames.size()); ++i)
        {
            const bool selected = (m_selectedDevice == i);
            if (ImGui::Selectable(deviceNames[i].c_str(), selected))
            {
                m_selectedDevice = i;
            }
            if (selected)
            {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // Send mode selector
    ImGui::Combo("Mode", &m_sendMode, "Raw Text\0Hex Bytes\0Binary Frame\0");

    ImGui::Separator();

    const std::string& targetDevice = deviceNames[m_selectedDevice];

    if (m_sendMode == 0) // Raw Text
    {
        ImGui::Combo("Line Ending", &m_lineEnding, "\\r\\n (Windows)\0\\n (Unix)\0\\r (Mac)\0None\0");

        const bool enterPressed = ImGui::InputText("##rawinput", m_textInput, sizeof(m_textInput), ImGuiInputTextFlags_EnterReturnsTrue);

        ImGui::SameLine();
        if (ImGui::Button("Send") || enterPressed)
        {
            std::string text(m_textInput);
            if (!text.empty())
            {
                switch (m_lineEnding)
                {
                    case 0: text += "\r\n"; break;
                    case 1: text += "\n"; break;
                    case 2: text += "\r"; break;
                    default: break;
                }

                std::vector<uint8_t> bytes(text.begin(), text.end());
                m_deviceMgr->sendRaw(targetDevice, bytes);

                m_history.push_back({targetDevice, std::format("TEXT: {}", m_textInput)});
                m_textInput[0] = '\0';
            }
        }

        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Type a command and press Enter or click Send");
        }
    }
    else if (m_sendMode == 1) // Hex Bytes
    {
        const bool enterPressed = ImGui::InputText("##hexinput", m_textInput, sizeof(m_textInput), ImGuiInputTextFlags_EnterReturnsTrue);
        if (ImGui::IsItemHovered())
        {
            ImGui::SetTooltip("Hex bytes separated by spaces, e.g.: AA 0B FF 01\n"
                              "Also accepts 0xAA, commas, or colons as separators");
        }

        ImGui::SameLine();
        if (ImGui::Button("Send") || enterPressed)
        {
            auto bytes = parseHexString(std::string(m_textInput));
            if (!bytes.empty())
            {
                m_deviceMgr->sendRaw(targetDevice, bytes);

                std::string display = std::format("HEX [{}B]:", bytes.size());
                for (auto b : bytes)
                {
                    display += std::format(" {:02X}", b);
                }
                m_history.push_back({targetDevice, display});
                m_textInput[0] = '\0';
            }
        }
    }
    else // Binary Frame
    {
        ImGui::InputInt("Channel", &m_channel);
        m_channel = std::clamp(m_channel, 0, 255);

        ImGui::InputDouble("Value", &m_value, 0.0, 0.0, "%.6f");

        if (ImGui::Button("Send"))
        {
            DataFrame frame{};
            frame.channel = static_cast<uint16_t>(m_channel);
            frame.timestamp = 0.0;
            frame.value = m_value;

            m_deviceMgr->sendCommand(targetDevice, frame);

            m_history.push_back({targetDevice, std::format("FRAME: ch={} val={:.6f}", m_channel, m_value)});
        }
    }

    // Cap history
    if (m_history.size() > 100)
    {
        m_history.erase(m_history.begin());
    }

    ImGui::Separator();
    ImGui::Text("History");

    if (ImGui::BeginChild("CmdHistory", ImVec2(0, 0), ImGuiChildFlags_None, ImGuiWindowFlags_HorizontalScrollbar))
    {
        for (const auto&[device, text] : std::ranges::reverse_view(m_history))
        {
            ImGui::TextWrapped("[%s] %s", device.c_str(), text.c_str());
        }
    }
    ImGui::EndChild();

    ImGui::End();
}
