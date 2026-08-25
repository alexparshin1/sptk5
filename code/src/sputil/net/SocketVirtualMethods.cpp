/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-03-03                                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/Printer.h>
#include <sptk5/SystemException.h>
#include <sptk5/net/SocketVirtualMethods.h>

#ifndef _WIN32
#include <sys/ioctl.h>
#include <sys/poll.h>
#endif

#include <fcntl.h>
#include <fstream>
#include <thread>
#include <sstream> // libc++

using namespace std;

namespace sptk {
SocketVirtualMethods::SocketVirtualMethods(const SOCKET_ADDRESS_FAMILY domain, const int32_t type, const int32_t protocol)
    : m_domain(domain)
    , m_type(type)
    , m_protocol(protocol)
{
}

void SocketVirtualMethods::openUnlocked(const Host&, OpenMode, bool, const chrono::milliseconds&, const char*)
{
    // Implement in derived class
}

void SocketVirtualMethods::openUnlocked(const sockaddr_in& address, const OpenMode openMode, const bool blockMode, const std::chrono::milliseconds& timeoutMS, const char* clientBindAddress)
{
    openAddressUnlocked(address, openMode, timeoutMS, true, clientBindAddress);
    setBlockingModeUnlocked(blockMode);
}

bool SocketVirtualMethods::activeUnlocked() const
{
    return m_socketFd != INVALID_SOCKET;
}

void SocketVirtualMethods::openAddressUnlocked(const sockaddr_in& addr, const OpenMode openMode,
                                               const chrono::milliseconds& timeout, const bool reusePort,
                                               const char* clientBindAddress, const int backlog)
{
    const auto timeoutMS = static_cast<int>(timeout.count());

    if (m_socketFd != INVALID_SOCKET)
    {
        closeUnlocked();
    }

    // Create a new socket
    m_socketFd = socket(m_domain, m_type, m_protocol);
    if (m_socketFd == INVALID_SOCKET)
    {
        throwSocketError("Can't create socket");
    }

    // A newly created socket is blocking. Reset the cached mode so the first
    // setBlockingModeUnlocked(false) is not skipped as an apparent no-op.
    m_blockingMode = true;

    auto result = 0;
    auto currentOperation = "connect";

    switch (openMode)
    {
        case OpenMode::CONNECT:
            if (clientBindAddress != nullptr && clientBindAddress[0] != 0)
            {
                bindUnlocked(clientBindAddress, 0, reusePort);
            }

            if (timeoutMS != 0)
            {
                setBlockingModeUnlocked(false);
                result = connect(m_socketFd, bit_cast<const sockaddr*>(&addr), sizeof(sockaddr_in));
                if (result == -1)
                {
                    switch (getSocketError())
                    {
                        case ENETUNREACH:
                            throw Exception("Network unreachable");
                        case ECONNREFUSED:
                            throw Exception("Connection refused");
                        default:
                            break;
                    }
                }
                try
                {
#ifdef _WIN32
                    // Windows processes non-blocking connect completion lazily: the blocking
                    // poll wake-up and the SO_ERROR query both stall for 1-2 system timer
                    // ticks (8-25ms measured on loopback) while the completion "settles",
                    // even though a zero-timeout WSAPoll sees the established state within
                    // microseconds and the socket is immediately usable. Probe the state
                    // directly and skip the SO_ERROR query on a clean success: SO_ERROR is
                    // only needed when the probe reports an error condition (a refused
                    // connect raises POLLWRNORM together with POLLERR|POLLHUP), and by then
                    // it no longer stalls.
                    auto cleanConnectSeen = false;
                    {
                        WSAPOLLFD connectPoll {};
                        connectPoll.fd = m_socketFd;
                        connectPoll.events = POLLWRNORM;
                        for (const auto spinDeadline = chrono::steady_clock::now() + min(timeout, chrono::milliseconds(2));
                             chrono::steady_clock::now() < spinDeadline;)
                        {
                            connectPoll.revents = 0;
                            if (WSAPoll(&connectPoll, 1, 0) > 0 && connectPoll.revents != 0)
                            {
                                break;
                            }
                            this_thread::yield();
                        }

                        if (connectPoll.revents == 0)
                        {
                            // Slow (remote) connect: fall back to the blocking wait. By the
                            // time it wakes, the completion has settled and SO_ERROR is cheap.
                            if (!readyToWriteUnlocked(timeout))
                            {
                                throw Exception("Connection timeout");
                            }
                        }
                        else
                        {
                            cleanConnectSeen = (connectPoll.revents & POLLWRNORM) != 0 &&
                                               (connectPoll.revents & (POLLERR | POLLHUP)) == 0;
                        }
                    }
                    if (!cleanConnectSeen)
                    {
                        getOptionUnlocked(SOL_SOCKET, SO_ERROR, result);
                        if (result != 0)
                        {
                            throwSocketError("Can't connect");
                        }
                    }
#else
                    if (!readyToWriteUnlocked(timeout))
                    {
                        throw Exception("Connection timeout");
                    }
                    getOptionUnlocked(SOL_SOCKET, SO_ERROR, result);
                    if (result != 0)
                    {
                        throwSocketError("Can't connect");
                    }
#endif
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
                // A blocking connect() can be interrupted by a signal (EINTR) well before any
                // real connection failure - observed under FastTcpServerTests.acceptRate, where
                // 8 threads opening 1000 connections concurrently hit this often enough to abort
                // the test with an uncaught exception. Retry rather than treating it as a failure.
                do
                {
                    result = connect(m_socketFd, bit_cast<const sockaddr*>(&addr), sizeof(sockaddr_in));
                } while (result == -1 && getSocketError() == EINTR);
            }
            break;

        case OpenMode::BIND:
            if (reusePort)
            {
                // SO_REUSEADDR lets the listener bind over a socket lingering in TIME_WAIT - for
                // example, an ephemeral client port from an earlier connection that happens to fall
                // on this well-known port - which would otherwise fail with EADDRINUSE.
                setOptionUnlocked(SOL_SOCKET, SO_REUSEADDR, 1);
#ifndef _WIN32
                // SO_REUSEPORT additionally lets several listener threads share the same port for
                // kernel-side load balancing of incoming connections. Windows has no SO_REUSEPORT;
                // there SO_REUSEADDR already covers shared binding.
                setOptionUnlocked(SOL_SOCKET, SO_REUSEPORT, 1);
#endif
            }
            currentOperation = "bind";
            result = ::bind(m_socketFd, bit_cast<const sockaddr*>(&addr), sizeof(sockaddr_in));
            if (result == 0 && m_type != SOCK_DGRAM)
            {
                result = listen(m_socketFd, backlog);
                currentOperation = "listen";
            }
            break;

        case OpenMode::CREATE:
            break;
    }

    if (result != 0)
    {
        stringstream error;
        error << "Can't " << currentOperation;
        // open(sockaddr_in) never sets m_host, so the failure path must not assume one:
        // dereferencing it here would replace the real error with a crash.
        if (m_host)
        {
            error << " to " << m_host->toString(false);
        }
        error << ". " << SystemException::osError() << ".";
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
        close(m_socketFd);
#else
        closesocket(m_socketFd);
#endif
        m_socketFd = INVALID_SOCKET;
    }
}

void SocketVirtualMethods::shutdownUnlocked() const noexcept
{
    const SocketType socketFd = m_socketFd.load();
    if (socketFd == INVALID_SOCKET)
    {
        return;
    }
#ifndef _WIN32
    (void) shutdown(socketFd, SHUT_RDWR);
#else
    (void) shutdown(socketFd, SD_BOTH);
#endif
}

SocketType SocketVirtualMethods::getSocketFdUnlocked() const
{
    return m_socketFd;
}

void SocketVirtualMethods::setSocketFdUnlocked(const SocketType socket)
{
    m_socketFd = socket;
}

void SocketVirtualMethods::setHostUnlocked(const Host& host)
{
    m_host = make_unique<Host>(host);
}

const Host& SocketVirtualMethods::getHostUnlocked() const
{
    if (!m_host)
    {
        throw Exception("Host is not set");
    }
    return *m_host;
}

bool SocketVirtualMethods::getBlockingModeUnlocked() const
{
    return m_blockingMode;
}

void SocketVirtualMethods::setBlockingModeUnlocked(const bool blockingMode)
{
    static const String errorMessage("Can't set socket blockingMode mode");

    if (m_blockingMode == blockingMode)
    {
        return;
    }
#ifdef _WIN32
    u_long arg = blockingMode ? 0 : 1;
    if (const int result = ioctlsocket(m_socketFd, FIONBIO, &arg);
        result != 0)
    {
        throwSocketError(errorMessage);
    }
#else
    auto flags = fcntl(m_socketFd, F_GETFL);
    if ((flags & O_NONBLOCK) == O_NONBLOCK)
    {
        flags -= O_NONBLOCK;
    }

    if (!blockingMode)
    {
        flags |= O_NONBLOCK;
    }

    if (const auto result = fcntl(m_socketFd, F_SETFL, flags);
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

void SocketVirtualMethods::setOptionUnlocked(const int level, const int option, int value) const
{
    constexpr socklen_t optionLen = sizeof(int);
    if (setsockopt(m_socketFd, level, option, VALUE_TYPE(&value), optionLen) != 0)
    {
        throwSocketError("Can't set socket option");
    }
}

void SocketVirtualMethods::getOptionUnlocked(const int level, const int option, int& value) const
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
        const auto result = ioctl(m_socketFd, FIONREAD, &bytes);
#endif
        result < 0)
    {
        return 0;
    }
    return bytes;
}

void SocketVirtualMethods::attachUnlocked(const SocketType socketHandle, bool)
{
    if (activeUnlocked())
    {
        closeUnlocked();
    }
    m_socketFd = socketHandle;

    // A descriptor returned by accept(), or handed over by a proxy, is blocking: neither
    // Linux nor Windows propagates O_NONBLOCK to it. Reset the cached mode to match, or the
    // first setBlockingModeUnlocked(false) is skipped as an apparent no-op and the socket
    // silently stays blocking - which then parks a reader in recv() while it holds the lock.
    m_blockingMode = true;
}

SocketType SocketVirtualMethods::detachUnlocked()
{
    const SocketType socketFd = m_socketFd.load();
    m_socketFd = INVALID_SOCKET;
    closeUnlocked();
    return socketFd;
}

// Guarded exactly as its only caller in bindUnlocked() is: IP_BIND_ADDRESS_NO_PORT is Linux-only,
// and without it this helper is dead code that -Wall reports as an unused function on FreeBSD.
#if !defined(_WIN32) && defined(IP_BIND_ADDRESS_NO_PORT)
namespace {

// IP_LOCAL_PORT_RANGE arrived in Linux 6.3. Define it when the build host's headers are
// older: the option number is ABI, and an older kernel just answers ENOPROTOOPT.
#ifndef IP_LOCAL_PORT_RANGE
constexpr int IP_LOCAL_PORT_RANGE = 51;
#endif

/// @brief Reads net.ipv4.ip_local_port_range, packed the way IP_LOCAL_PORT_RANGE wants it:
/// low port in the low 16 bits, high port in the high 16 bits.
/// @returns Packed range, or 0 if it could not be read or made no sense.
uint32_t localPortRange()
{
    static const uint32_t range = []() -> uint32_t
    {
        unsigned low = 0;
        unsigned high = 0;
        if (ifstream file("/proc/sys/net/ipv4/ip_local_port_range"); file >> low >> high)
        {
            if (low > 0 && high >= low && high <= 65535)
            {
                return static_cast<uint32_t>(high) << 16U | static_cast<uint32_t>(low);
            }
        }
        return 0;
    }();
    return range;
}

} // namespace
#endif

void SocketVirtualMethods::bindUnlocked(const char* address, const uint32_t portNumber, const bool reusePort)
{
    if (m_socketFd == INVALID_SOCKET)
    {
        // Create a new socket
        m_socketFd = socket(m_domain, m_type, m_protocol);
        if (m_socketFd == INVALID_SOCKET)
        {
            throwSocketError("Can't create socket");
        }

        // A newly created socket is blocking; keep the cached mode in step, as
        // openAddressUnlocked() does.
        m_blockingMode = true;
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

#if !defined(_WIN32) && defined(IP_BIND_ADDRESS_NO_PORT)
    if (portNumber == 0)
    {
        // Binding a source address with port 0 - "this address, any port" - is the client
        // case: pick the local interface, don't care which port. Left alone, the kernel
        // must choose the port during bind(), before it knows the destination, so it has
        // to prove (address, port) is free against every other binding by walking the
        // bhash2 conflict chains. Those chains grow with the number of bound sockets, so
        // each connect costs O(chain length) - quadratic overall.
        //
        // Measured on a 1M-connection run: inet_bind2_bucket_match_addr_any and its
        // callers went from ~8% of client CPU at 500K connections to ~95% at 950K, and
        // the connect rate collapsed from 5000/s to ~70/s.
        //
        // IP_BIND_ADDRESS_NO_PORT defers port selection to connect(), where the full
        // 4-tuple is known and the cheap __inet_check_established path applies - and it
        // lets the same port be reused toward different destinations.
        //
        // Deliberately not setOptionUnlocked(): that throws on failure, which would turn a
        // kernel that does not support the option into a failed bind. This is an
        // optimisation, not a requirement - ignore the result and carry on.
        constexpr int enable = 1;
        (void) ::setsockopt(m_socketFd, IPPROTO_IP, IP_BIND_ADDRESS_NO_PORT, &enable, sizeof(enable));

        // Second half of the same problem. __inet_hash_connect() walks candidate ports
        // with a step of 2, taking only one parity - the other half is left to
        // bind()-time allocation - so IP_BIND_ADDRESS_NO_PORT on its own can reach only
        // 32256 of the 64512 ports per source address. Setting IP_LOCAL_PORT_RANGE, to
        // whatever the netns range already is, flips that step to 1 and makes the whole
        // range reachable.
        //
        // Measured from a single source address: without this the connect rate holds
        // ~135000/s up to 32245 connections and then collapses to ~740/s, because every
        // later connect scans the whole exhausted parity before trying the other one.
        // With it, 60000 connections from one address stay at ~126000/s.
        //
        // The parity split exists to stop connect() and bind(0) competing for the same
        // ports. Taking the full range is the right trade for a socket that only ever
        // allocates at connect() time - exactly the portNumber == 0 case here.
        if (const uint32_t range = localPortRange(); range != 0)
        {
            (void) ::setsockopt(m_socketFd, IPPROTO_IP, IP_LOCAL_PORT_RANGE, &range, sizeof(range));
        }
    }
#endif

    if (::bind(m_socketFd, bit_cast<sockaddr*>(&addr), sizeof(addr)) != 0)
    {
        throwSocketError("Can't bind socket to port " + to_string(portNumber));
    }
}

void SocketVirtualMethods::listenUnlocked(const uint16_t portNumber, const bool reusePort, const int backlog)
{
    if (portNumber != 0)
    {
        m_host = make_unique<Host>(getHostUnlocked().hostname(), portNumber);
    }

    sockaddr_in address = {};

    memset(&address, 0, sizeof(address));
    address.sin_family = static_cast<SOCKET_ADDRESS_FAMILY>(m_domain);
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(getHostUnlocked().port());

    openAddressUnlocked(address, OpenMode::BIND, chrono::milliseconds(0), reusePort, nullptr, backlog);
}

#if (__FreeBSD__ | __OpenBSD__)
constexpr int CONNECTION_CLOSED = POLLHUP;
#else
#ifdef _WIN32
constexpr int CONNECTION_CLOSED = POLLHUP;
#else
constexpr auto CONNECTION_CLOSED = POLLRDHUP | POLLHUP;
#endif
#endif

bool SocketVirtualMethods::readyToReadUnlocked(const chrono::milliseconds& timeout)
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
            if (fdArray.revents & CONNECTION_CLOSED)
            {
                throw ConnectionException("Connection closed");
            }
            break;
        default:
            throw ConnectionException("Connection closed");
    }
    return false;
#else
    pollfd pfd = {};

    pfd.fd = m_socketFd;
    pfd.events = POLLIN;
    const auto result = poll(&pfd, 1, timeoutMS);

    if (result < 0)
    {
        throwSocketError("Can't read from socket");
    }

    if (result == 1 && (pfd.revents & (CONNECTION_CLOSED | POLLERR | POLLNVAL)) != 0)
    {
        throw ConnectionException("Connection closed");
    }

    return result != 0;
#endif
}

bool SocketVirtualMethods::readyToWriteUnlocked(const chrono::milliseconds& timeout)
{
    const auto timeoutMS = static_cast<int>(timeout.count());

    // A closed socket is never ready, and asking the OS about it is worse than useless:
    // WSAPoll() on INVALID_SOCKET falls into the error branch below and reports an unrelated
    // error code. readyToReadUnlocked() guards the same way.
    if (m_socketFd == INVALID_SOCKET)
    {
        return false;
    }

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
    }
    return false;
#else
    pollfd pfd = {};
    pfd.fd = m_socketFd;
    pfd.events = POLLOUT;
    const auto result = poll(&pfd, 1, timeoutMS);
    if (result < 0)
    {
        throwSocketError("Can't read from socket");
    }

    if (result == 1 && (pfd.revents & CONNECTION_CLOSED) != 0)
    {
        throw Exception("Connection closed");
    }

    return result != 0;
#endif
}

size_t SocketVirtualMethods::recvUnlocked(uint8_t* buffer, const size_t len)
{
    const int result = recv(m_socketFd, bit_cast<char*>(buffer), static_cast<int32_t>(len), 0);
    if (result == -1)
    {
        if (const auto error = getSocketError();
            error == EAGAIN || error == EINTR || error == EINPROGRESS)
        {
            return RECV_RETRY;
        }
        throwSocketError("Can't read from socket");
    }
    return static_cast<size_t>(result);
}

size_t SocketVirtualMethods::readUnlocked(uint8_t* buffer, const size_t size, sockaddr* from)
{
    if (size == 0)
    {
        return 0;
    }

#ifndef _WIN32
    ssize_t bytes;
#else
    int bytes;
#endif

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

size_t SocketVirtualMethods::sendUnlocked(const uint8_t* buffer, const size_t len)
{
    while (true)
    {
#ifdef _WIN32
        auto res = send(m_socketFd, bit_cast<char*>(buffer), static_cast<int32_t>(len), 0);
#else
        auto res = send(m_socketFd, bit_cast<char*>(buffer), static_cast<int32_t>(len), MSG_NOSIGNAL);
#endif
        if (res == -1)
        {
            if (m_socketFd == INVALID_SOCKET)
            {
                throwSocketError("Socket is closed");
            }
            if (auto error = getSocketError();
                error == EAGAIN || error == EINTR || error == EINPROGRESS)
            {
                if (!readyToWriteUnlocked(10s))
                {
                    throw ConnectionException("Can't write to socket: timeout");
                }
                continue;
            }
            throwSocketError("Can't write to socket");
        }
        return res;
    }
}

size_t SocketVirtualMethods::writeUnlocked(const uint8_t* buffer, size_t size, const sockaddr* peer)
{
    const auto* ptr = buffer;

    if (static_cast<int>(size) == -1)
    {
        size = strlen(bit_cast<const char*>(buffer));
    }

    const auto total = size;
    auto       remaining = static_cast<ssize_t>(size);
    while (remaining > 0)
    {
        ssize_t bytes;
        if (peer != nullptr)
        {
            // UDP socket
#ifdef _WIN32
            bytes = sendto(m_socketFd, bit_cast<const char*>(ptr), remaining, 0,
                           bit_cast<const sockaddr*>(peer),
                           sizeof(sockaddr_in));
#else
            bytes = sendto(m_socketFd, bit_cast<const char*>(ptr), remaining, MSG_NOSIGNAL,
                           bit_cast<const sockaddr*>(peer),
                           sizeof(sockaddr_in));
#endif
            if (bytes == -1)
            {
                throwSocketError("Can't write to socket");
            }
        }
        else
        {
            bytes = static_cast<int>(sendUnlocked(ptr, remaining));
        }

        remaining -= bytes;
        ptr += bytes;
    }
    return total;
}

int32_t SocketVirtualMethods::getDomainUnlocked() const
{
    return m_domain;
}

int32_t SocketVirtualMethods::getTypeUnlocked() const
{
    return m_type;
}

int32_t SocketVirtualMethods::getProtocolUnlocked() const
{
    return m_protocol;
}

int getSocketError(int nativeErrorCode)
{
#ifdef _WIN32
    switch (nativeErrorCode == -1 ? GetLastError() : nativeErrorCode)
    {
        case 0:
            return 0;
        case WSA_INVALID_HANDLE:
        case WSAEBADF:
            return EBADF;
        case WSAENOTSOCK:
            return ENOTSOCK;
        case WSAENOTCONN:
            return ENOTCONN;
        case WSAENETUNREACH:
            return ENETUNREACH;
        case WSAECONNABORTED:
            return ECONNABORTED;
        case WSAECONNRESET:
            return ECONNRESET;
        case WSAECONNREFUSED:
            return ECONNREFUSED;
        case WSAEWOULDBLOCK:
        case EINPROGRESS:
            return EAGAIN;
        default:
            break;
    }
    return EBADF;
#else
    return errno;
#endif
}

void throwSocketError(const String& message, const std::source_location& location)
{
    string errorStr;

#ifdef _WIN32
    constexpr int               maxMessageSize {256};
    array<char, maxMessageSize> buffer {};

    const auto windowsSocketError = static_cast<int>(GetLastError());
    if (windowsSocketError != 0)
    {
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, windowsSocketError, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer.data(), maxMessageSize, nullptr);
        errorStr = buffer.data();
    }

    int error = getSocketError(windowsSocketError);
#else
    auto error = errno;
    // strerror_r() doesn't work here
    errorStr = strerror(errno);
#endif
    switch (error)
    {
        case ENOTCONN:
            throw ConnectionException(message + ": Connection is not open", location);
        case EPIPE:
        case EBADF:
            throw ConnectionException(message + ": Connection is closed", location);
        case ECONNABORTED:
        case ECONNRESET:
            throw ConnectionException(message + ": Connection is terminated", location);
        case ECONNREFUSED:
            throw ConnectionException(message + ": Connection is refused", location);
        case EAGAIN:
        case EINPROGRESS:
            throw RepeatOperationException(message + ": " + errorStr, location);
        case EMFILE:
            throw ConnectionException(message + ": Too many open files", location);
        default:
            break;
    }

    if (errno != 0)
    {
        CERR("ERRNO is " << errno);
    }

    if (!errorStr.empty())
    {
        throw ConnectionException(message + ": " + errorStr, location);
    }
    throw ConnectionException(message + ": Unknown error", location);
}

} // namespace sptk
