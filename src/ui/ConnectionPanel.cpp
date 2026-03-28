#include "ui/ConnectionPanel.h"

#include "core/DeviceManager.h"
#include "core/IProtocolParser.h"
#include "core/ITransport.h"
#include "core/SerialTransport.h"
#include "core/TransportFactory.h"

#include <imgui.h>
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

#include <algorithm>
#include <cstdio>

using namespace embview::ui;
using namespace embview::core;

ConnectionPanel::ConnectionPanel(std::shared_ptr<core::DeviceManager> deviceMgr)
    : m_deviceMgr(std::move(deviceMgr))
{
    refreshPorts();
}

ConnectionPanel::~ConnectionPanel() = default;

void ConnectionPanel::render(bool& open)
{
    if (!ImGui::Begin("Connection", &open))
    {
        ImGui::End();
        return;
    }

    // Check if an async connection attempt completed
    if (m_connecting && m_connectFuture.valid())
    {
        auto status = m_connectFuture.wait_for(std::chrono::milliseconds(0));
        if (status == std::future_status::ready)
        {
            bool success = m_connectFuture.get();
            m_connecting = false;
            if (success)
            {
                m_statusMessage = "Connected successfully!";
                m_statusIsError = false;
            }
            else
            {
                m_statusMessage = "Connection failed. Check log for details.";
                m_statusIsError = true;
            }
            m_statusTimer = 5.0;
        }
    }

    // Device name
    ImGui::InputText("Device Name", m_deviceName, sizeof(m_deviceName));

    // Transport type
    ImGui::Combo("Transport", &m_transportType, "Serial\0TCP\0UDP\0");

    // Protocol mode
    ImGui::Combo("Protocol", &m_protocolMode, "Binary\0ASCII Line\0ASCII CSV\0");
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip(
            "Binary: [0xAA][LEN][CH][double][CRC8]\n"
            "ASCII Line: \"value\\n\" or \"channel:value\\n\"\n"
            "ASCII CSV: \"v0,v1,v2,...\\n\"");
    }

    ImGui::Separator();

    if (m_transportType == 0) // Serial
    {
        if (ImGui::Button("Refresh Ports"))
        {
            refreshPorts();
        }

        ImGui::SameLine();

        if (!m_availablePorts.empty())
        {
            if (ImGui::BeginCombo("Port", m_availablePorts[m_selectedPort].c_str()))
            {
                for (int i = 0; i < static_cast<int>(m_availablePorts.size()); ++i)
                {
                    bool isSelected = (m_selectedPort == i);
                    if (ImGui::Selectable(m_availablePorts[i].c_str(), isSelected))
                    {
                        m_selectedPort = i;
                    }
                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }
        else
        {
            ImGui::TextDisabled("No ports available");
        }

        // Baud rate
        {
            std::string baudLabel = std::to_string(BAUD_RATES[m_selectedBaud]);
            if (ImGui::BeginCombo("Baud Rate", baudLabel.c_str()))
            {
                for (int i = 0; i < BAUD_RATE_COUNT; ++i)
                {
                    std::string label = std::to_string(BAUD_RATES[i]);
                    bool isSelected = (m_selectedBaud == i);
                    if (ImGui::Selectable(label.c_str(), isSelected))
                    {
                        m_selectedBaud = i;
                    }
                    if (isSelected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }
    }
    else if (m_transportType == 1) // TCP
    {
        ImGui::InputText("Host", m_tcpHost, sizeof(m_tcpHost));
        ImGui::InputInt("Port", &m_tcpPort);
        m_tcpPort = std::clamp(m_tcpPort, 1, 65535);
    }
    else // UDP
    {
        ImGui::InputText("Bind Address", m_udpHost, sizeof(m_udpHost));
        ImGui::InputInt("Port", &m_udpPort);
        m_udpPort = std::clamp(m_udpPort, 1, 65535);
        ImGui::Checkbox("Broadcast", &m_udpBroadcast);
    }

    ImGui::Separator();

    // Connect button
    bool canConnect = !m_connecting && ((m_transportType == 0) ? !m_availablePorts.empty() : true);
    if (!canConnect)
    {
        ImGui::BeginDisabled();
    }

    if (ImGui::Button(m_connecting ? "Connecting..." : "Connect"))
    {
        try
        {
            nlohmann::json config;
            std::string type;

            if (m_transportType == 0)
            {
                type = "serial";
                config["port"] = m_availablePorts[m_selectedPort];
                config["baud"] = BAUD_RATES[m_selectedBaud];
            }
            else if (m_transportType == 1)
            {
                type = "tcp";
                config["host"] = std::string(m_tcpHost);
                config["port"] = static_cast<uint16_t>(m_tcpPort);
            }
            else
            {
                type = "udp";
                config["host"] = std::string(m_udpHost);
                config["port"] = static_cast<uint16_t>(m_udpPort);
                config["broadcast"] = m_udpBroadcast;
            }

            auto transport = std::shared_ptr<ITransport>(
                TransportFactory::instance().create(type, config).release());

            auto protoMode = static_cast<ProtocolMode>(m_protocolMode);
            std::string name(m_deviceName);

            // TCP and UDP connect asynchronously to avoid freezing the UI
            if (m_transportType != 0)
            {
                m_connecting = true;
                m_statusMessage = "Connecting...";
                m_statusIsError = false;
                m_statusTimer = 0.0; // Don't auto-clear while connecting

                auto devMgr = m_deviceMgr;

                m_connectFuture = std::async(std::launch::async,
                    [transport, protoMode, name, devMgr]() -> bool
                    {
                        if (!transport->open())
                        {
                            spdlog::error("Failed to open connection for {}", name);
                            return false;
                        }
                        devMgr->addDevice(name, transport, protoMode);
                        return true;
                    });

                m_deviceCounter++;
                std::snprintf(m_deviceName, sizeof(m_deviceName), "Device-%d", m_deviceCounter);
            }
            else
            {
                // Serial is generally fast, connect synchronously
                if (transport->open())
                {
                    m_deviceMgr->addDevice(name, transport, protoMode);
                    m_deviceCounter++;
                    std::snprintf(m_deviceName, sizeof(m_deviceName), "Device-%d", m_deviceCounter);
                    m_statusMessage = "Connected successfully!";
                    m_statusIsError = false;
                    m_statusTimer = 5.0;
                }
                else
                {
                    m_statusMessage = "Failed to open serial port.";
                    m_statusIsError = true;
                    m_statusTimer = 5.0;
                    spdlog::error("Failed to open serial connection");
                }
            }
        }
        catch (const std::exception& e)
        {
            m_statusMessage = std::string("Error: ") + e.what();
            m_statusIsError = true;
            m_statusTimer = 5.0;
            spdlog::error("Connection error: {}", e.what());
        }
    }

    if (!canConnect)
    {
        ImGui::EndDisabled();
    }

    // Status message display
    if (!m_statusMessage.empty())
    {
        ImGui::SameLine();
        if (m_connecting)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.85f, 0.2f, 1.0f), "%s", m_statusMessage.c_str());
        }
        else if (m_statusIsError)
        {
            ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "%s", m_statusMessage.c_str());
        }
        else
        {
            ImGui::TextColored(ImVec4(0.3f, 0.85f, 0.45f, 1.0f), "%s", m_statusMessage.c_str());
        }

        // Auto-clear status after timer expires
        if (m_statusTimer > 0.0 && !m_connecting)
        {
            m_statusTimer -= ImGui::GetIO().DeltaTime;
            if (m_statusTimer <= 0.0)
            {
                m_statusMessage.clear();
            }
        }
    }

    ImGui::Separator();

    // Connected devices list
    ImGui::Text("Connected Devices:");

    auto names = m_deviceMgr->getDeviceNames();
    if (names.empty())
    {
        ImGui::TextDisabled("None");
    }
    else
    {
        for (const auto& name : names)
        {
            bool connected = m_deviceMgr->isDeviceConnected(name);
            if (connected)
            {
                ImGui::TextColored(ImVec4(0.3f, 0.85f, 0.45f, 1.0f), "%s", name.c_str());
            }
            else
            {
                ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "%s (disconnected)", name.c_str());
            }
            ImGui::SameLine();
            ImGui::PushID(name.c_str());
            if (ImGui::SmallButton("Remove"))
            {
                m_deviceMgr->removeDevice(name);
                ImGui::PopID();
                break;
            }
            ImGui::PopID();
        }
    }

    ImGui::End();
}

void ConnectionPanel::refreshPorts()
{
    m_availablePorts = SerialTransport::listPorts();
    if (m_selectedPort >= static_cast<int>(m_availablePorts.size()))
    {
        m_selectedPort = 0;
    }
}
