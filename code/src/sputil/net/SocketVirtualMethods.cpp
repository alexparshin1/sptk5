/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
╚══════════════════════════════════════════════════════════════════════════════╝
┌──────────────────────────────────────────────────────────────────────────────┐
│   This library is free software; you can redistribute it and/or modify it    │
│   under the terms of the GNU Library General Public License as published by  │
│   the Free Software Foundation; either version 2 of the License, or (at your │
│   option) any later version.                                                 │
│                                                                              │
│   This library is distributed in the hope that it will be useful, but        │
│   WITHOUT ANY WARRANTY; without even the implied warranty of                 │
│   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library   │
│   General Public License for more details.                                   │
│                                                                              │
│   You should have received a copy of the GNU Library General Public License  │
│   along with this library; if not, write to the Free Software Foundation,    │
│   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/Printer.h>
#include <sptk5/SystemException.h>
#include <sptk5/net/SocketVirtualMethods.h>

#ifndef _WIN32

#include <sys/poll.h>

#endif

using namespace std;

namespace sptk {

SocketVirtualMethods::SocketVirtualMethods(SOCKET_ADDRESS_FAMILY domain, int32_t type, int32_t protocol)
    : m_domain(domain)
    , m_type(type)
    , m_protocol(protocol)
{
}

void SocketVirtualMethods::openAddressUnlocked(const sockaddr_in& addr, OpenMode openMode,
                                               std::chrono::milliseconds timeout, bool reusePort)
{
    const auto timeoutMS = static_cast<int>(timeout.count());

    if (activeUnlocked())
    {
        closeUnlocked();
    }

    // Create a new socket
    m_socketFd = socket(m_domain, m_type, m_protocol);
    if (m_socketFd == INVALID_SOCKET)
    {
        throwSocketError("Can't create socket");
    }

    int result = 0;
    auto currentOperation = "connect";

    switch (openMode)
    {
        case OpenMode::CONNECT:
            if (timeoutMS != 0)
            {
                setBlockingModeUnlocked(false);
                result = connect(m_socketFd, bit_cast<const sockaddr*>(&addr), sizeof(sockaddr_in));
                switch (result)
                {
                    case ENETUNREACH:
                        throw Exception("Network unreachable");
                    case ECONNREFUSED:
                        throw Exception("Connection refused");
                    default:
                        break;
                }
                try
                {
                    if (!readyToWriteUnlocked(timeout))
                    {
                        throw Exception("Connection timeout");
                    }
                }
                catch (const Exception&)
                {
                    closeUnlocked();
                    throw;
                }
                result = 0;
                setBlockingModeUnlocked(true);
            }
            else
            {
                result = connect(m_socketFd, bit_cast<const sockaddr*>(&addr), sizeof(sockaddr_in));
            }
            break;

        case OpenMode::BIND:
            if (reusePort)
            {
#ifndef _WIN32
                setOptionUnlocked(SOL_SOCKET, SO_REUSEPORT, 1);
#else
                setOptionUnlocked(SOL_SOCKET, SO_REUSEADDR, 1);
#endif
            }
            currentOperation = "bind";
            result = bind(m_socketFd, bit_cast<const sockaddr*>(&addr), sizeof(sockaddr_in));
            if (result == 0 && m_type != SOCK_DGRAM)
            {
                result = listen(m_socketFd, SOMAXCONN);
                currentOperation = "listen";
            }
            break;

        default:
            break;
    }

    if (result != 0)
    {
        stringstream error;
        error << "Can't " << currentOperation << " to " << m_host.toString(false) << ". "
              << SystemException::osError()
              << ".";
        closeUnlocked();
        throw Exception(error.str());
    }
}

void SocketVirtualMethods::closeUnlocked()
{
    if (m_socketFd != INVALID_SOCKET)
    {
#ifndef _WIN32
        shutdown(m_socketFd, SHUT_RDWR);
        ::close(m_socketFd);
#else
        closesocket(m_socketFd);
#endif
        m_socketFd = INVALID_SOCKET;
    }
}

void SocketVirtualMethods::setBlockingModeUnlocked(bool blockingMode)
{
    const String errorMessage("Can't set socket blockingMode mode");
#ifdef _WIN32
    u_long arg = blockingMode ? 0 : 1;
    if (const int result = ioctlsocket(m_socketFd, FIONBIO, &arg);
        result != 0)
    {
        throwSocketError(errorMessage);
    }
#else
    int flags = fcntl(m_socketFd, F_GETFL);
    if ((flags & O_NONBLOCK) == O_NONBLOCK)
    {
        flags -= O_NONBLOCK;
    }

    if (!blockingMode)
    {
        flags |= O_NONBLOCK;
    }

    if (const int result = fcntl(m_socketFd, F_SETFL, flags);
        result != 0)
    {
        throwSocketError(errorMessage);
    }
#endif

    m_blockingMode = blockingMode;
}

#ifdef _WIN32
#define VALUE_TYPE(val) bit_cast<char*>((val))
#else
#define VALUE_TYPE(val) bit_cast<void*>((val))
#endif

void SocketVirtualMethods::setOptionUnlocked(int level, int option, int value) const
{
    constexpr socklen_t len = sizeof(int);
    if (setsockopt(m_socketFd, level, option, VALUE_TYPE(&value), len) != 0)
    {
        throwSocketError("Can't set socket option");
    }
}

void SocketVirtualMethods::getOptionUnlocked(int level, int option, int& value) const
{
    socklen_t len = sizeof(int);
    if (getsockopt(m_socketFd, level, option, VALUE_TYPE(&value), &len) != 0)
    {
        throwSocketError("Can't get socket option");
    }
}

size_t SocketVirtualMethods::getSocketBytesUnlocked() const
{
    uint32_t bytes = 0;
    if (
#ifdef _WIN32
        const int32_t result = ioctlsocket(m_socketFd, FIONREAD, bit_cast<u_long*>(&bytes));
#else
        const int32_t result = ioctl(m_socketFd, FIONREAD, &bytes);
#endif
        result < 0)
    {
        return 0;
    }
    return bytes;
}

void SocketVirtualMethods::attachUnlocked(SocketType socketHandle, bool)
{
    if (activeUnlocked())
    {
        closeUnlocked();
    }
    m_socketFd = socketHandle;
}

SocketType SocketVirtualMethods::detachUnlocked()
{
    const SocketType socketFd = m_socketFd;
    m_socketFd = INVALID_SOCKET;
    closeUnlocked();
    return socketFd;
}

void SocketVirtualMethods::bindUnlocked(const char* address, uint32_t portNumber, bool reusePort)
{
    if (m_socketFd == INVALID_SOCKET)
    {
        // Create a new socket
        m_socketFd = socket(m_domain, m_type, m_protocol);
        if (m_socketFd == INVALID_SOCKET)
        {
            throwSocketError("Can't create socket");
        }
    }

    sockaddr_in addr = {};
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = static_cast<SOCKET_ADDRESS_FAMILY>(m_domain);

    if (address == nullptr)
    {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        addr.sin_addr.s_addr = inet_addr(address);
    }

    addr.sin_port = htons(static_cast<uint16_t>(portNumber));

    if (reusePort)
    {
#ifdef _WIN32
        setOptionUnlocked(SOL_SOCKET, SO_REUSEADDR, 1);
#else
        setOptionUnlocked(SOL_SOCKET, SO_REUSEPORT, 1);
#endif
    }

    if (bind(m_socketFd, bit_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        throwSocketError("Can't bind socket to port " + int2string(portNumber));
    }
}

void SocketVirtualMethods::listenUnlocked(uint16_t portNumber, bool reusePort)
{
    if (portNumber != 0)
    {
        m_host.port(portNumber);
    }

    sockaddr_in addr = {};

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = static_cast<SOCKET_ADDRESS_FAMILY>(m_domain);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(m_host.port());

    openAddressUnlocked(addr, OpenMode::BIND, chrono::milliseconds(0), reusePort);
}

#if (__FreeBSD__ | __OpenBSD__)
constexpr int CONNCLOSED = POLLHUP;
#else
#ifdef _WIN32
constexpr int CONNCLOSED = POLLHUP;
#else
constexpr int CONNCLOSED = POLLRDHUP | POLLHUP;
#endif
#endif

bool SocketVirtualMethods::readyToReadUnlocked(chrono::milliseconds timeout)
{
    const auto timeoutMS = static_cast<int>(timeout.count());

    if (m_socketFd == INVALID_SOCKET)
    {
        return false;
    }

#ifdef _WIN32
    WSAPOLLFD fdArray {};
    fdArray.fd = m_socketFd;
    fdArray.events = POLLRDNORM;
    switch (WSAPoll(&fdArray, 1, timeoutMS))
    {
        case 0:
            return false;
        case 1:
            if (fdArray.revents & POLLRDNORM)
            {
                return true;
            }
            if (fdArray.revents & POLLHUP)
            {
                throw ConnectionException("Connection closed");
            }
            break;
        default:
            throw ConnectionException("Connection closed");
    }
    return false;
#else
    struct pollfd pfd = {};

    pfd.fd = m_socketFd;
    pfd.events = POLLIN;
    const int result = poll(&pfd, 1, timeoutMS);
    if (result < 0)
    {
        throwSocketError("Can't read from socket");
    }
    if (result == 1 && (pfd.revents & CONNCLOSED) != 0)
    {
        throw ConnectionException("Connection closed");
    }
    return result != 0;
#endif
}

bool SocketVirtualMethods::readyToWriteUnlocked(std::chrono::milliseconds timeout)
{
    const auto timeoutMS = static_cast<int>(timeout.count());
#ifdef _WIN32
    WSAPOLLFD fdArray {};
    fdArray.fd = m_socketFd;
    fdArray.events = POLLWRNORM;
    switch (WSAPoll(&fdArray, 1, timeoutMS))
    {
        case 0:
            return false;
        case 1:
            if (fdArray.revents & POLLWRNORM)
            {
                return true;
            }
            if (fdArray.revents & POLLHUP)
            {
                throw ConnectionException("Connection closed");
            }
            break;
        default:
            throwSocketError("WSAPoll error");
            break;
    }
    return false;
#else
    struct pollfd pfd = {};
    pfd.fd = m_socketFd;
    pfd.events = POLLOUT;
    const int result = poll(&pfd, 1, timeoutMS);
    if (result < 0)
    {
        throwSocketError("Can't read from socket");
    }

    if (result == 1 && (pfd.revents & CONNCLOSED) != 0)
    {
        throw Exception("Connection closed");
    }

    return result != 0;
#endif
}

size_t SocketVirtualMethods::recvUnlocked(uint8_t* buffer, size_t len)
{
#ifdef _WIN32
    auto result = recv(m_socketFd, bit_cast<char*>(buffer), static_cast<int32_t>(len), 0);
#else
    auto result = ::recv(m_socketFd, bit_cast<char*>(buffer), (int32_t) len, MSG_DONTWAIT);
#endif
    if (result == -1)
    {
        constexpr chrono::seconds timeout(30);
        if (readyToReadUnlocked(timeout))
        {
            result = recv(m_socketFd, bit_cast<char*>(buffer), static_cast<int32_t>(len), 0);
        }
    }
    return static_cast<size_t>(result);
}

size_t SocketVirtualMethods::readUnlocked(uint8_t* buffer, size_t size, sockaddr_in* from)
{
    int bytes;
    if (from != nullptr)
    {
        socklen_t fromLength = sizeof(sockaddr_in);
        bytes = recvfrom(m_socketFd, bit_cast<char*>(buffer), static_cast<int32_t>(size), 0,
                         bit_cast<sockaddr*>(from), &fromLength);
    }
    else
    {
        bytes = recv(m_socketFd, bit_cast<char*>(buffer), static_cast<int32_t>(size), 0);
    }

    if (bytes == -1)
    {
        throwSocketError("Can't read from socket");
    }

    return static_cast<size_t>(bytes);
}

size_t SocketVirtualMethods::sendUnlocked(const uint8_t* buffer, size_t len)
{
#ifdef _WIN32
    auto res = send(m_socketFd, bit_cast<char*>(buffer), static_cast<int32_t>(len), 0);
#else
    auto res = ::send(m_socketFd, bit_cast<char*>(buffer), (int32_t) len, MSG_NOSIGNAL);
#endif
    return res;
}

size_t SocketVirtualMethods::writeUnlocked(const uint8_t* buffer, size_t size, const sockaddr_in* peer)
{
    const auto* ptr = buffer;

    if (static_cast<int>(size) == -1)
    {
        size = strlen(bit_cast<const char*>(buffer));
    }

    const size_t total = size;
    auto remaining = static_cast<int>(size);
    while (remaining > 0)
    {
        int bytes;
        if (peer != nullptr)
        {
#ifdef _WIN32
            bytes = sendto(m_socketFd, bit_cast<const char*>(ptr), static_cast<int32_t>(size), 0,
                           bit_cast<const sockaddr*>(peer),
                           sizeof(sockaddr_in));
#else
            bytes = (int) sendto(m_socketFd, bit_cast<const char*>(ptr), (int32_t) size, MSG_NOSIGNAL,
                                 bit_cast<const sockaddr*>(peer),
                                 sizeof(sockaddr_in));
#endif
        }
        else
        {
            bytes = static_cast<int>(sendUnlocked(ptr, static_cast<int32_t>(size)));
        }

        if (bytes == -1)
        {
            throwSocketError("Can't write to socket");
        }

        remaining -= bytes;
        ptr += bytes;
    }
    return total;
}

void throwSocketError(const String& message, const std::source_location& location)
{
    string errorStr;

#ifdef _WIN32
    constexpr int maxMessageSize {256};
    array<char, maxMessageSize> buffer {};

    const DWORD dw = GetLastError();

    if (dw != 0)
    {
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer.data(), maxMessageSize, nullptr);
        errorStr = buffer.data();
    }
#else
    // strerror_r() doesn't work here
    errorStr = strerror(errno);
#endif
    switch (errno)
    {
        case EPIPE:
        case EBADF:
            throw ConnectionException(message + ": Connection is closed", location);
        case EAGAIN:
        case EINPROGRESS:
            throw RepeatOperationException(message + ": " + errorStr, location);
        default:
            break;
    }

    CERR("ERRNO is " << errno << endl);

    if (!errorStr.empty())
    {
        throw Exception(message + ": " + errorStr, location);
    }
}

} // namespace sptk
