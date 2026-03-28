#pragma once

#include "core/ITransport.h"
#include "core/SocketGuard.h"

#include <cstdint>
#include <string>
#include <vector>

namespace embview::core
{
    /// @brief Configuration for UDP socket connections.
    struct UdpConfig
    {
        std::string host = "0.0.0.0";
        uint16_t port = 5000;
        bool broadcast = false;
    };

    /**
     * @brief UDP transport for datagram-based device communication.
     *
     * Binds to a local port and receives datagrams. Supports broadcast
     * reception for devices that multicast their data. Non-blocking reads.
     * Uses RAII for socket and Winsock lifetime management.
     */
    class UdpTransport : public ITransport
    {
    public:
        explicit UdpTransport(UdpConfig config);
        ~UdpTransport() override = default;

        UdpTransport(UdpTransport&&) noexcept = default;
        UdpTransport& operator=(UdpTransport&&) noexcept = default;

        UdpTransport(const UdpTransport&) = delete;
        UdpTransport& operator=(const UdpTransport&) = delete;

        bool open() override;
        void close() override;
        bool isOpen() const override;
        std::vector<uint8_t> read(std::size_t maxBytes) override;
        std::size_t write(std::span<const uint8_t> data) override;

    private:
        UdpConfig m_config;
        bool m_isOpen = false;
        WsaGuard m_wsa;
        SocketGuard m_socket;
    };
} // namespace embview::core
