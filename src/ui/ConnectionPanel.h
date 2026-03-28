#pragma once

#include <atomic>
#include <future>
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
    * @brief Panel for managing device connections (Serial, TCP, UDP).
    *
    * Supports adding multiple simultaneous connections. Each connection
    * runs its own reader thread via the DeviceManager.
    * TCP/UDP connections are attempted asynchronously to avoid freezing the UI.
    */
    class ConnectionPanel
    {
    public:
        explicit ConnectionPanel(std::shared_ptr<core::DeviceManager> deviceMgr);
        ~ConnectionPanel();

        void render(bool& open);

    private:
        void refreshPorts();

        std::shared_ptr<core::DeviceManager> m_deviceMgr;

        // Transport type: 0=Serial, 1=TCP, 2=UDP
        int m_transportType = 0;

        // Protocol mode: 0=Binary, 1=ASCII Line, 2=ASCII CSV
        int m_protocolMode = 1;

        // Serial fields
        std::vector<std::string> m_availablePorts;
        int m_selectedPort = 0;
        int m_selectedBaud = 4;

        // TCP fields
        char m_tcpHost[64] = "192.168.1.1";
        int m_tcpPort = 5000;

        // UDP fields
        char m_udpHost[64] = "0.0.0.0";
        int m_udpPort = 5000;
        bool m_udpBroadcast = false;

        // Device name
        char m_deviceName[64] = "Device-1";
        int m_deviceCounter = 1;

        // Connection status
        std::string m_statusMessage;
        bool m_statusIsError = false;
        double m_statusTimer = 0.0;
        bool m_connecting = false;
        std::future<bool> m_connectFuture;

        static constexpr int BAUD_RATES[] = {9600, 19200, 38400, 57600, 115200, 230400, 460800, 921600};
        static constexpr int BAUD_RATE_COUNT = 8;
    };
} // namespace embview::ui
