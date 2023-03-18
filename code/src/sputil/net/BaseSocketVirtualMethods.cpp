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

#include "sptk5/net/BaseSocketVirtualMethods.h"

#ifndef _WIN32
#include <sys/poll.h>
#endif

using namespace std;

namespace sptk {

BaseSocketVirtualMethods::BaseSocketVirtualMethods(SOCKET_ADDRESS_FAMILY domain, int32_t type, int32_t protocol)
    : m_domain(domain)
    , m_type(type)
    , m_protocol(protocol)
{
}

void BaseSocketVirtualMethods::closeUnlocked()
{
    if (m_sockfd != INVALID_SOCKET)
    {
#ifndef _WIN32
        shutdown(m_sockfd, SHUT_RDWR);
        ::close(m_sockfd);
#else
        closesocket(m_sockfd);
#endif
        m_sockfd = INVALID_SOCKET;
    }
}

void BaseSocketVirtualMethods::setBlockingModeUnlocked(bool blockingMode)
{
#ifdef _WIN32
    uint32_t arg = blockingMode ? 0 : 1;
    control(FIONBIO, &arg);
    u_long arg2 = arg;
    const int result = ioctlsocket(m_sockfd, FIONBIO, &arg2);
#else
    int flags = fcntl(m_sockfd, F_GETFL);
    if ((flags & O_NONBLOCK) == O_NONBLOCK)
    {
        flags -= O_NONBLOCK;
    }
    if (!blockingMode)
    {
        flags |= O_NONBLOCK;
    }
    const int result = fcntl(m_sockfd, F_SETFL, flags);
#endif
    if (result != 0)
    {
        throwSocketError("Can't set socket blockingMode mode");
    }

    m_blockingMode = blockingMode;
}

#ifdef _WIN32
#define VALUE_TYPE(val) (char*) (val)
#else
#define VALUE_TYPE(val) (void*) (val)
#endif

void BaseSocketVirtualMethods::setOptionUnlocked(int level, int option, int value) const
{
    const socklen_t len = sizeof(int);
    if (setsockopt(m_sockfd, level, option, VALUE_TYPE(&value), len) != 0)
        throwSocketError("Can't set socket option");
}

void BaseSocketVirtualMethods::getOptionUnlocked(int level, int option, int& value) const
{
    socklen_t len = sizeof(int);
    if (getsockopt(m_sockfd, level, option, VALUE_TYPE(&value), &len) != 0)
        throwSocketError("Can't get socket option");
}

size_t BaseSocketVirtualMethods::getSocketBytesUnlocked() const
{
    uint32_t bytes = 0;
    if (
#ifdef _WIN32
        const int32_t result = ioctlsocket(m_sockfd, FIONREAD, (u_long*) &bytes);
#else
        const int32_t result = ioctl(m_sockfd, FIONREAD, &bytes);
#endif
        result < 0)
        throwSocketError("Can't get socket bytes");

    return bytes;
}

void BaseSocketVirtualMethods::attachUnlocked(SOCKET socketHandle, bool)
{
    if (activeUnlocked())
    {
        closeUnlocked();
    }
    m_sockfd = socketHandle;
}

SOCKET BaseSocketVirtualMethods::detachUnlocked()
{
    SOCKET sockfd = m_sockfd;
    m_sockfd = INVALID_SOCKET;
    closeUnlocked();
    return sockfd;
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

bool BaseSocketVirtualMethods::readyToReadUnlocked(chrono::milliseconds timeout)
{
    const auto timeoutMS = (int) timeout.count();

    if (m_sockfd == INVALID_SOCKET)
    {
        return false;
    }

#ifdef _WIN32
    WSAPOLLFD fdarray {};
    fdarray.fd = m_sockfd;
    fdarray.events = POLLRDNORM;
    int result = WSAPoll(&fdarray, 1, timeoutMS);
    switch (result)
    {
        case 0:
            return false;
        case 1:
            if (fdarray.revents & POLLRDNORM)
                return true;
            if (fdarray.revents & POLLHUP)
                throw ConnectionException("Connection closed");
            break;
        default:
            throwSocketError("WSAPoll error");
            break;
    }
    return false;
#else
    struct pollfd pfd = {};

    pfd.fd = m_sockfd;
    pfd.events = POLLIN;
    int result = poll(&pfd, 1, timeoutMS);
    if (result < 0)
        throwSocketError("Can't read from socket");
    if (result == 1 && (pfd.revents & CONNCLOSED) != 0)
    {
        throw ConnectionException("Connection closed");
    }
    return result != 0;
#endif
}

bool BaseSocketVirtualMethods::readyToWriteUnlocked(std::chrono::milliseconds timeout)
{
    const auto timeoutMS = (int) timeout.count();
#ifdef _WIN32
    WSAPOLLFD fdarray {};
    fdarray.fd = m_sockfd;
    fdarray.events = POLLWRNORM;
    switch (WSAPoll(&fdarray, 1, timeoutMS))
    {
        case 0:
            return false;
        case 1:
            if (fdarray.revents & POLLWRNORM)
                return true;
            if (fdarray.revents & POLLHUP)
                throw ConnectionException("Connection closed");
            break;
        default:
            throwSocketError("WSAPoll error");
            break;
    }
    return false;
#else
    struct pollfd pfd = {};
    pfd.fd = m_sockfd;
    pfd.events = POLLOUT;
    int result = poll(&pfd, 1, timeoutMS);
    if (result < 0)
        throwSocketError("Can't read from socket");
    if (result == 1 && (pfd.revents & CONNCLOSED) != 0)
    {
        throw Exception("Connection closed");
    }
    return result != 0;
#endif
}

void throwSocketError(const String& operation, const std::source_location& location)
{
    string errorStr;
#ifdef _WIN32
    constexpr int maxMessageSize {256};
    array<char, maxMessageSize> buffer {};

    LPCTSTR lpMsgBuf = nullptr;
    const DWORD dw = GetLastError();
    if (dw != 0)
    {
        FormatMessage(
            FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
            nullptr, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) buffer.data(), maxMessageSize, nullptr);
        errorStr = buffer.data();
    }
#else
    // strerror_r() doesn't work here
    errorStr = strerror(errno);
#endif
    if (!errorStr.empty())
    {
        throw Exception(operation + ": " + errorStr, location);
    }
}

} // namespace sptk
