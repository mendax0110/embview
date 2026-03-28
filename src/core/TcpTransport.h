#pragma once

#include "core/ITransport.h"
#include "core/SocketGuard.h"

#include <cstdint>
#include <string>
#include <vector>

namespace embview::core
{
    /// @brief Configuration for TCP socket connections.
    struct TcpConfig
    {
        std::string host = "192.168.1.1";
        uint16_t port = 5000;
    };

    /**
     * @brief TCP transport for Ethernet-connected devices.
     *
     * Connects to a remote host:port via TCP and provides non-blocking
     * read/write over the connection. Uses RAII for socket and Winsock
     * lifetime management.
     */
    class TcpTransport : public ITransport
    {
    public:
        explicit TcpTransport(TcpConfig config);
        ~TcpTransport() override = default;

        TcpTransport(TcpTransport&&) noexcept = default;
        TcpTransport& operator=(TcpTransport&&) noexcept = default;

        TcpTransport(const TcpTransport&) = delete;
        TcpTransport& operator=(const TcpTransport&) = delete;

        bool open() override;
        void close() override;
        bool isOpen() const override;
        std::vector<uint8_t> read(std::size_t maxBytes) override;
        std::size_t write(std::span<const uint8_t> data) override;

    private:
        TcpConfig m_config;
        bool m_isOpen = false;
        WsaGuard m_wsa;
        SocketGuard m_socket;
    };
} // namespace embview::core
