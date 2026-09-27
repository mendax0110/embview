#pragma once

#include "core/ITransport.h"
#include "core/SocketGuard.h"

#include <cstdint>
#include <string>
#include <vector>

/**
 * @namespace embview::core Core support.
 */
namespace embview::core
{
    /**
     * @brief Configuration for TCP socket connections. \struct TcpConfig
     */
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
        /**
         * @brief Create a TCP transport with the given configuration.
         * @param config TCP connection settings.
         */
        explicit TcpTransport(TcpConfig config);
        ~TcpTransport() override = default;

        TcpTransport(TcpTransport&&) noexcept = default;
        TcpTransport& operator=(TcpTransport&&) noexcept = default;

        TcpTransport(const TcpTransport&) = delete;
        TcpTransport& operator=(const TcpTransport&) = delete;

        /**
         * @brief Open the TCP socket connection.
         * @return true on success.
         */
        bool open() override;

        /**
         * @brief Close the TCP socket.
         */
        void close() override;

        /**
         * @brief Check whether the TCP connection is open.
         * @return true when connected.
         */
        [[nodiscard]] bool isOpen() const override;

        /**
         * @brief Read up to @p maxBytes bytes from the socket.
         * @param maxBytes Maximum number of bytes to read.
         * @return Bytes received from the peer.
         */
        std::vector<uint8_t> read(std::size_t maxBytes) override;

        /**
         * @brief Write data to the TCP socket.
         * @param data Bytes to send.
         * @return Number of bytes written.
         */
        std::size_t write(std::span<const uint8_t> data) override;

    private:
        TcpConfig m_config;
        bool m_isOpen = false;
        WsaGuard m_wsa;
        SocketGuard m_socket;
    };
} // namespace embview::core
