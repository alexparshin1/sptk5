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

#include <cerrno>
#include <sptk5/SystemException.h>
#include <sptk5/net/BaseSocket.h>

using namespace std;
using namespace sptk;

#ifdef _WIN32
static int m_socketCount;
static bool m_inited(false);
#endif

#ifdef _WIN32
void BaseSocket::init() noexcept
{
    if (m_inited)
        return;
    m_inited = true;
    WSADATA wsaData = {};
    const WORD wVersionRequested = MAKEWORD(2, 0);
    WSAStartup(wVersionRequested, &wsaData);
}

void BaseSocket::cleanup() noexcept
{
    m_inited = false;
    WSACleanup();
}
#endif

// Constructor
BaseSocket::BaseSocket(SOCKET_ADDRESS_FAMILY domain, int32_t type, int32_t protocol)
    : BaseSocketVirtualMethods(domain, type, protocol)
{
#ifdef _WIN32
    init();
    m_socketCount++;
#endif
}

BaseSocket::~BaseSocket()
{
    BaseSocket::close();
}

size_t BaseSocket::recvUnlocked(uint8_t* buffer, size_t len)
{
#ifdef _WIN32
    auto result = ::recv(m_sockfd, (char*) buffer, (int32_t) len, 0);
#else
    auto result = ::recv(m_socketFd, (char*) buffer, (int32_t) len, MSG_DONTWAIT);
#endif
    if (result == -1)
    {
        constexpr chrono::seconds timeout(30);
        if (readyToReadUnlocked(timeout))
        {
            result = ::recv(m_socketFd, (char*) buffer, (int32_t) len, 0);
        }
    }
    return (size_t) result;
}

size_t BaseSocket::send(const uint8_t* buffer, size_t len)
{
    auto res = ::send(m_socketFd, (const char*) buffer, (int32_t) len, 0);
    return res;
}

int32_t BaseSocket::control(int flag, const uint32_t* check) const
{
#ifdef _WIN32
    return ioctlsocket(m_sockfd, flag, (u_long*) check);
#else
    return fcntl(m_socketFd, flag, *check);
#endif
}

void BaseSocket::bind(const char* address, uint32_t portNumber)
{
    if (m_socketFd == INVALID_SOCKET)
    {
        // Create a new socket
        m_socketFd = socket(m_domain, m_type, m_protocol);
        if (m_socketFd == INVALID_SOCKET)
            throwSocketError("Can't create socket");
    }

    sockaddr_in addr = {};
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = (SOCKET_ADDRESS_FAMILY) m_domain;

    if (address == nullptr)
    {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
        addr.sin_addr.s_addr = inet_addr(address);
    }

    addr.sin_port = htons(uint16_t(portNumber));

    if (::bind(m_socketFd, (sockaddr*) &addr, sizeof(addr)) != 0)
        throwSocketError("Can't bind socket to port " + int2string(portNumber));
}

void BaseSocket::listen(uint16_t portNumber)
{
    if (portNumber != 0)
    {
        m_host.port(portNumber);
    }

    sockaddr_in addr = {};

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = (SOCKET_ADDRESS_FAMILY) m_domain;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(m_host.port());

    openAddressUnlocked(addr, OpenMode::BIND);
}

size_t BaseSocket::read(Buffer& buffer, size_t size, sockaddr_in* from)
{
    const std::scoped_lock lock(m_socketMutex);

    buffer.checkSize(size);
    size_t bytes = readUnlocked(buffer.data(), size, from);
    buffer.bytes(bytes);

    return bytes;
}

size_t BaseSocket::read(String& buffer, size_t size, sockaddr_in* from)
{
    const std::scoped_lock lock(m_socketMutex);

    buffer.resize(size);
    size_t bytes = readUnlocked((uint8_t*) buffer.data(), size, from);
    buffer.resize(bytes);

    return bytes;
}

size_t BaseSocket::write(const uint8_t* buffer, size_t size, const sockaddr_in* peer)
{
    int bytes;
    const auto* ptr = buffer;

    if ((int) size == -1)
    {
        size = strlen((const char*) buffer);
    }

    const size_t total = size;
    auto remaining = (int) size;
    while (remaining > 0)
    {
        if (peer != nullptr)
        {
            bytes = (int) sendto(m_socketFd, (const char*) ptr, (int32_t) size, 0, (const sockaddr*) peer,
                                 sizeof(sockaddr_in));
        }
        else
        {
            bytes = (int) send(ptr, (int32_t) size);
        }
        if (bytes == -1)
            throwSocketError("Can't write to socket");
        remaining -= bytes;
        ptr += bytes;
    }
    return total;
}

size_t BaseSocket::write(const Buffer& buffer, const sockaddr_in* peer)
{
    return write(buffer.data(), buffer.bytes(), peer);
}

size_t BaseSocket::write(const String& buffer, const sockaddr_in* peer)
{
    return write((const uint8_t*) buffer.c_str(), buffer.length(), peer);
}
