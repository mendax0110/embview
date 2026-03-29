#include "core/TcpTransport.h"

#include <spdlog/spdlog.h>

using namespace embview::core;

static constexpr int kConnectTimeoutSec = 3;

TcpTransport::TcpTransport(TcpConfig config)
    : m_config(std::move(config))
{
}

bool TcpTransport::open()
{
    if (m_isOpen)
    {
        return true;
    }

    if (!m_wsa.init())
    {
        return false;
    }

    m_socket = SocketGuard::createTcp();
    if (!m_socket)
    {
        spdlog::error("Failed to create TCP socket");
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_config.port);

    if (inet_pton(AF_INET, m_config.host.c_str(), &addr.sin_addr) <= 0)
    {
        spdlog::error("Invalid TCP host address: {}", m_config.host);
        m_socket.reset();
        return false;
    }

    if (!m_socket.connectWithTimeout(addr, kConnectTimeoutSec))
    {
        spdlog::error("TCP connect timed out or failed to {}:{}", m_config.host, m_config.port);
        m_socket.reset();
        return false;
    }

    m_isOpen = true;
    spdlog::info("TCP connected to {}:{}", m_config.host, m_config.port);
    return true;
}

void TcpTransport::close()
{
    if (!m_isOpen && !m_socket)
    {
        return;
    }

    m_isOpen = false;
    m_socket.reset();
    spdlog::info("TCP disconnected from {}:{}", m_config.host, m_config.port);
}

bool TcpTransport::isOpen() const
{
    return m_isOpen;
}

std::vector<uint8_t> TcpTransport::read(const std::size_t maxBytes)
{
    if (!m_isOpen)
    {
        return {};
    }

    std::vector<uint8_t> buffer(maxBytes);
    const int result = m_socket.recv(buffer.data(), maxBytes);

    if (result == -1) // Would block
    {
        return {};
    }
    if (result == -2) // Error
    {
        spdlog::warn("TCP recv error");
        return {};
    }
    if (result == 0) // Remote closed
    {
        spdlog::info("TCP connection closed by remote");
        m_isOpen = false;
        return {};
    }

    buffer.resize(static_cast<std::size_t>(result));
    return buffer;
}

std::size_t TcpTransport::write(const std::span<const uint8_t> data)
{
    if (!m_isOpen)
    {
        return 0;
    }

    std::size_t sent = m_socket.send(data.data(), data.size());
    if (sent == 0)
    {
        spdlog::warn("TCP send error");
    }
    return sent;
}
