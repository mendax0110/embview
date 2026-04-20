#pragma once

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <ws2tcpip.h>
#else
    #include <arpa/inet.h>
    #include <errno.h>
    #include <fcntl.h>
    #include <netinet/in.h>
    #include <sys/socket.h>
    #include <unistd.h>
#endif

#include <spdlog/spdlog.h>

#include <cstdint>
#include <utility>

namespace embview::core
{
    /// @brief RAII wrapper for Winsock initialization (WSAStartup / WSACleanup).
    /// On non-Windows platforms this is a no-op.
    class WsaGuard
    {
    public:
        WsaGuard() = default;

        /// @brief Initializes Winsock. Returns false on failure.
        bool init()
        {
#ifdef _WIN32
            if (m_initialized)
            {
                return true;
            }
            WSADATA wsaData;
            if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
            {
                spdlog::error("WSAStartup failed");
                return false;
            }
            m_initialized = true;
#endif
            return true;
        }

        ~WsaGuard()
        {
#ifdef _WIN32
            if (m_initialized)
            {
                WSACleanup();
            }
#endif
        }

        WsaGuard(WsaGuard&& other) noexcept
#ifdef _WIN32
            : m_initialized(std::exchange(other.m_initialized, false))
#endif
        {
        }

        WsaGuard& operator=(WsaGuard&& other) noexcept
        {
            if (this != &other)
            {
#ifdef _WIN32
                if (m_initialized)
                {
                    WSACleanup();
                }
                m_initialized = std::exchange(other.m_initialized, false);
#endif
            }
            return *this;
        }

        WsaGuard(const WsaGuard&) = delete;
        WsaGuard& operator=(const WsaGuard&) = delete;

    private:
#ifdef _WIN32
        bool m_initialized = false;
#endif
    };

    /// @brief RAII wrapper for a platform socket handle.
    /// Closes the socket on destruction. Move-only.
    class SocketGuard
    {
    public:
#ifdef _WIN32
        using NativeHandle = SOCKET;
        static constexpr NativeHandle kInvalid = INVALID_SOCKET;
#else
        using NativeHandle = int;
        static constexpr NativeHandle kInvalid = -1;
#endif

        SocketGuard() = default;

        explicit SocketGuard(NativeHandle sock) : m_socket(sock) {}

        ~SocketGuard()
        {
            reset();
        }

        SocketGuard(SocketGuard&& other) noexcept
            : m_socket(std::exchange(other.m_socket, kInvalid))
        {
        }

        SocketGuard& operator=(SocketGuard&& other) noexcept
        {
            if (this != &other)
            {
                reset();
                m_socket = std::exchange(other.m_socket, kInvalid);
            }
            return *this;
        }

        SocketGuard(const SocketGuard&) = delete;
        SocketGuard& operator=(const SocketGuard&) = delete;

        /// @brief Returns the native socket handle.
        [[nodiscard]] NativeHandle get() const { return m_socket; }

        /// @brief Returns true if the socket is valid (not closed / never opened).
        [[nodiscard]] bool valid() const { return m_socket != kInvalid; }

        /// @brief Implicit bool: true when holding a valid socket.
        explicit operator bool() const { return valid(); }

        /// @brief Releases ownership and returns the raw handle without closing.
        NativeHandle release()
        {
            return std::exchange(m_socket, kInvalid);
        }

        /// @brief Closes the current socket (if valid) and takes ownership of a new one.
        void reset(const NativeHandle newSock = kInvalid)
        {
            if (m_socket != kInvalid)
            {
#ifdef _WIN32
                ::closesocket(m_socket);
#else
                ::close(m_socket);
#endif
            }
            m_socket = newSock;
        }

        /// @brief Sets the socket to non-blocking mode.
        void setNonBlocking() const
        {
            if (m_socket == kInvalid)
            {
                return;
            }
#ifdef _WIN32
            u_long mode = 1;
            ioctlsocket(m_socket, FIONBIO, &mode);
#else
            const int flags = fcntl(m_socket, F_GETFL, 0);
            fcntl(m_socket, F_SETFL, flags | O_NONBLOCK);
#endif
        }

        /// @brief Sets a socket option. Wraps setsockopt with platform differences.
        template <typename T>
        bool setOption(const int level, const int optName, const T& value)
        {
            if (m_socket == kInvalid)
            {
                return false;
            }
#ifdef _WIN32
            return ::setsockopt(m_socket, level, optName,
                                reinterpret_cast<const char*>(&value),
                                sizeof(value)) == 0;
#else
            return ::setsockopt(m_socket, level, optName, &value, sizeof(value)) == 0;
#endif
        }

        /// @brief Gets a socket option. Wraps getsockopt with platform differences.
        template <typename T>
        bool getOption(int level, int optName, T& value)
        {
            if (m_socket == kInvalid)
            {
                return false;
            }
#ifdef _WIN32
            int optLen = sizeof(value);
            return ::getsockopt(m_socket, level, optName,
                                reinterpret_cast<char*>(&value), &optLen) == 0;
#else
            socklen_t optLen = sizeof(value);
            return ::getsockopt(m_socket, level, optName, &value, &optLen) == 0;
#endif
        }

        /// @brief Sends data on a connected socket. Returns bytes sent or 0 on error.
        std::size_t send(const uint8_t* data, const std::size_t size) const
        {
#ifdef _WIN32
            int result = ::send(m_socket, reinterpret_cast<const char*>(data),
                                static_cast<int>(size), 0);
            if (result == SOCKET_ERROR)
            {
                return 0;
            }
            return static_cast<std::size_t>(result);
#else
            const ssize_t result = ::send(m_socket, data, size, 0);
            if (result < 0)
            {
                return 0;
            }
            return static_cast<std::size_t>(result);
#endif
        }

        /// @brief Receives data from a connected socket. Returns bytes read, 0 on close, -1 on would-block.
        int recv(uint8_t* buffer, const std::size_t maxBytes) const
        {
#ifdef _WIN32
            int result = ::recv(m_socket, reinterpret_cast<char*>(buffer),
                                static_cast<int>(maxBytes), 0);
            if (result == SOCKET_ERROR)
            {
                return (WSAGetLastError() == WSAEWOULDBLOCK) ? -1 : -2;
            }
            return result;
#else
            const ssize_t result = ::recv(m_socket, buffer, maxBytes, 0);
            if (result < 0)
            {
                return (errno == EAGAIN || errno == EWOULDBLOCK) ? -1 : -2;
            }
            return static_cast<int>(result);
#endif
        }

        /// @brief Receives a datagram. Returns bytes read, -1 on would-block, -2 on error.
        int recvFrom(uint8_t* buffer, const std::size_t maxBytes) const
        {
#ifdef _WIN32
            int result = ::recvfrom(m_socket, reinterpret_cast<char*>(buffer),
                                     static_cast<int>(maxBytes), 0, nullptr, nullptr);
            if (result == SOCKET_ERROR)
            {
                return (WSAGetLastError() == WSAEWOULDBLOCK) ? -1 : -2;
            }
            return result;
#else
            const ssize_t result = ::recvfrom(m_socket, buffer, maxBytes, 0, nullptr, nullptr);
            if (result < 0)
            {
                return (errno == EAGAIN || errno == EWOULDBLOCK) ? -1 : -2;
            }
            return static_cast<int>(result);
#endif
        }

        /// @brief Binds the socket to an address.
        [[nodiscard]] bool bind(const sockaddr_in& addr) const
        {
            if (m_socket == kInvalid)
            {
                return false;
            }
#ifdef _WIN32
            return ::bind(m_socket, reinterpret_cast<const sockaddr*>(&addr),
                          sizeof(addr)) != SOCKET_ERROR;
#else
            return ::bind(m_socket, reinterpret_cast<const sockaddr*>(&addr), sizeof(addr)) == 0;
#endif
        }

        /// @brief Non-blocking connect with timeout. Returns true if connected.
        bool connectWithTimeout(const sockaddr_in& addr, const int timeoutSec)
        {
            if (m_socket == kInvalid)
            {
                return false;
            }

            setNonBlocking();

#ifdef _WIN32
            int connectResult = ::connect(m_socket,
                                           reinterpret_cast<const sockaddr*>(&addr),
                                           sizeof(addr));

            if (connectResult == SOCKET_ERROR)
            {
                int err = WSAGetLastError();
                if (err != WSAEWOULDBLOCK)
                {
                    spdlog::error("connect() failed (error {})", err);
                    return false;
                }

                fd_set writeFds;
                FD_ZERO(&writeFds);
                FD_SET(m_socket, &writeFds);

                fd_set exceptFds;
                FD_ZERO(&exceptFds);
                FD_SET(m_socket, &exceptFds);

                timeval tv{};
                tv.tv_sec = timeoutSec;

                int sel = ::select(0, nullptr, &writeFds, &exceptFds, &tv);
                if (sel <= 0)
                {
                    return false;
                }

                if (FD_ISSET(m_socket, &exceptFds))
                {
                    int sockErr = 0;
                    getOption(SOL_SOCKET, SO_ERROR, sockErr);
                    spdlog::error("connect() SO_ERROR = {}", sockErr);
                    return false;
                }
            }
#else
            const int connectResult = ::connect(m_socket, reinterpret_cast<const sockaddr*>(&addr),sizeof(addr));

            if (connectResult < 0)
            {
                if (errno != EINPROGRESS)
                {
                    spdlog::error("connect() failed (errno {})", errno);
                    return false;
                }

                fd_set writeFds;
                FD_ZERO(&writeFds);
                FD_SET(m_socket, &writeFds);

                timeval tv{};
                tv.tv_sec = timeoutSec;

                int sel = ::select(m_socket + 1, nullptr, &writeFds, nullptr, &tv);
                if (sel <= 0)
                {
                    return false;
                }

                int sockErr = 0;
                getOption(SOL_SOCKET, SO_ERROR, sockErr);
                if (sockErr != 0)
                {
                    spdlog::error("connect() SO_ERROR = {}", sockErr);
                    return false;
                }
            }
#endif
            return true;
        }

        /// @brief Creates a TCP socket. Returns a SocketGuard that owns it.
        static SocketGuard createTcp()
        {
#ifdef _WIN32
            SOCKET sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
            return SocketGuard(sock);
#else
            return SocketGuard(::socket(AF_INET, SOCK_STREAM, 0));
#endif
        }

        /// @brief Creates a UDP socket. Returns a SocketGuard that owns it.
        static SocketGuard createUdp()
        {
#ifdef _WIN32
            SOCKET sock = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
            return SocketGuard(sock);
#else
            return SocketGuard(::socket(AF_INET, SOCK_DGRAM, 0));
#endif
        }

    private:
        NativeHandle m_socket = kInvalid;
    };
} // namespace embview::core
