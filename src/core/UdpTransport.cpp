#include "core/UdpTransport.h"

#include <spdlog/spdlog.h>

using namespace embview::core;

UdpTransport::UdpTransport(UdpConfig config)
    : m_config(std::move(config))
{
}

bool UdpTransport::open()
{
    if (m_isOpen)
    {
        return true;
    }

    if (!m_wsa.init())
    {
        return false;
    }

    m_socket = SocketGuard::createUdp();
    if (!m_socket)
    {
        spdlog::error("Failed to create UDP socket");
        return false;
    }

    if (m_config.broadcast)
    {
        int optval = 1;
        m_socket.setOption(SOL_SOCKET, SO_BROADCAST, optval);
    }

    {
        int optval = 1;
        m_socket.setOption(SOL_SOCKET, SO_REUSEADDR, optval);
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_config.port);

    if (inet_pton(AF_INET, m_config.host.c_str(), &addr.sin_addr) <= 0)
    {
        spdlog::error("Invalid UDP bind address: {}", m_config.host);
        m_socket.reset();
        return false;
    }

    if (!m_socket.bind(addr))
    {
        spdlog::error("UDP bind failed on {}:{}", m_config.host, m_config.port);
        m_socket.reset();
        return false;
    }

    m_socket.setNonBlocking();

    m_isOpen = true;
    spdlog::info("UDP listening on {}:{}", m_config.host, m_config.port);
    return true;
}

void UdpTransport::close()
{
    m_isOpen = false;
    m_socket.reset();
    spdlog::info("UDP socket closed");
}

bool UdpTransport::isOpen() const
{
    return m_isOpen;
}

std::vector<uint8_t> UdpTransport::read(std::size_t maxBytes)
{
    if (!m_isOpen)
    {
        return {};
    }

    std::vector<uint8_t> buffer(maxBytes);
    int result = m_socket.recvFrom(buffer.data(), maxBytes);

    if (result == -1) // Would block
    {
        return {};
    }
    if (result == -2) // Error
    {
        spdlog::warn("UDP recv error");
        return {};
    }

    buffer.resize(static_cast<std::size_t>(result));
    return buffer;
}

std::size_t UdpTransport::write(std::span<const uint8_t> /*data*/)
{
    if (!m_isOpen)
    {
        return 0;
    }

    spdlog::warn("UDP write not implemented (receiver-only mode)");
    return 0;
}
