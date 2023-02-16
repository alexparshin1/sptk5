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

#include <sptk5/RegularExpression.h>
#include <sptk5/SystemException.h>
#include <sptk5/net/BaseSocket.h>

#include <utility>

using namespace std;
using namespace sptk;

#ifdef _WIN32
static BaseSocket initializer; // Needed for WinSock2 initialization
#endif

Host::Host() noexcept
{
    memset(&m_address, 0, sizeof(m_address));
}

Host::Host(const String& hostname, uint16_t port)
    : m_hostname(hostname)
    , m_port(port)
{
    getHostAddress();
    setPort(m_port);
}

Host::Host(const String& hostAndPort)
{
    const RegularExpression matchHost(R"(^(\[.*\]|[^\[\]:]*)(:\d+)?)");
    auto matches = matchHost.m(hostAndPort);
    if (matches)
    {
        m_hostname = matches[0].value;
        if (matches.groups().size() > 1)
        {
            m_port = (uint16_t) string2int(matches[1].value.substr(1));
        }
        getHostAddress();
        setPort(m_port);
    }
    else
    {
        memset(&m_address, 0, sizeof(m_address));
    }
}

Host::Host(const sockaddr_in* addressAndPort)
{
    constexpr socklen_t addressLen = sizeof(sockaddr_in);
    memcpy(m_address.data(), addressAndPort, addressLen);
    m_port = htons(ip_v4().sin_port);

    setHostNameFromAddress(addressLen);
}

Host::Host(const sockaddr_in6* addressAndPort)
{
    constexpr socklen_t addressLen = sizeof(sockaddr_in6);

    const auto* addressAndPort6 = addressAndPort;
    memcpy((sockaddr_in6*) m_address.data(), addressAndPort6, addressLen);
    m_port = htons(ip_v6().sin6_port);

    setHostNameFromAddress(addressLen);
}

void Host::setHostNameFromAddress(socklen_t addressLen)
{
    array<char, NI_MAXHOST> hbuf {};
    array<char, NI_MAXSERV> sbuf {};
#ifdef _WIN32
    if (getnameinfo((const sockaddr*) m_address.data(), addressLen, hbuf.data(), sizeof(hbuf), sbuf.data(), sizeof(sbuf), 0) == 0)
        m_hostname = hbuf.data();
#else
    if (getnameinfo((const sockaddr*) m_address.data(), addressLen, hbuf.data(), sizeof(hbuf), sbuf.data(),
                    sizeof(sbuf), 0) ==
        0)
    {
        m_hostname = String(hbuf.data());
    }
#endif
}

Host::Host(const Host& other)
    : m_hostname(other.m_hostname)
    , m_port(other.m_port)
{
    const scoped_lock lock(other.m_mutex);
    memcpy(&m_address, &other.m_address, sizeof(m_address));
}

Host::Host(Host&& other) noexcept
    : m_hostname(exchange(other.m_hostname, ""))
    , m_port(exchange(other.m_port, 0))
{
    const scoped_lock lock(other.m_mutex);
    memcpy(&m_address, &other.m_address, sizeof(m_address));
}

Host& Host::operator=(const Host& other)
{
    if (&other != this)
    {
        const scoped_lock lock(m_mutex, other.m_mutex);
        m_hostname = other.m_hostname;
        m_port = other.m_port;
        memcpy(&m_address, &other.m_address, sizeof(m_address));
    }
    return *this;
}

Host& Host::operator=(Host&& other) noexcept
{
    const scoped_lock lock(m_mutex, other.m_mutex);
    m_hostname = other.m_hostname;
    m_port = other.m_port;
    memcpy(&m_address, &other.m_address, sizeof(m_address));
    return *this;
}

bool Host::operator==(const Host& other) const
{
    return toString(true) == other.toString(true);
}

bool Host::operator!=(const Host& other) const
{
    return toString(true) != other.toString(true);
}

void Host::setPort(uint16_t port)
{
    const scoped_lock lock(m_mutex);
    m_port = port;
    switch (any().sa_family)
    {
        case AF_INET:
            ip_v4().sin_port = htons(uint16_t(m_port));
            break;
        case AF_INET6:
            ip_v6().sin6_port = htons(uint16_t(m_port));
            break;
        default:
            break;
    }
}

void Host::getHostAddress()
{
#ifdef _WIN32
    struct hostent* host_info = gethostbyname(m_hostname.c_str());
    if (host_info == nullptr)
        throwSocketError("Can't get host info for " + m_hostname, __FILE__, __LINE__);

    UniqueLock(m_mutex);
    memset(&m_address, 0, sizeof(m_address));
    any().sa_family = host_info->h_addrtype;

    switch (any().sa_family)
    {
        case AF_INET:
            memcpy(&ip_v4().sin_addr, host_info->h_addr, size_t(host_info->h_length));
            break;
        case AF_INET6:
            memcpy(&ip_v6().sin6_addr, host_info->h_addr, size_t(host_info->h_length));
            break;
    }
#else
    const scoped_lock lock(m_mutex);

    struct addrinfo hints = {};
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;       // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // Socket type
    hints.ai_protocol = 0;

    struct addrinfo* result = nullptr;
    if (const int exitCode = getaddrinfo(m_hostname.c_str(), nullptr, &hints, &result);
        exitCode != 0)
    {
        throw Exception(gai_strerror(exitCode));
    }

    memset(&m_address, 0, sizeof(m_address));
    memcpy(&m_address, (struct sockaddr_in*) result->ai_addr, result->ai_addrlen);

    freeaddrinfo(result);
#endif
}

String Host::toString(bool forceAddress) const
{
    const scoped_lock lock(m_mutex);
    std::stringstream str;

    if (m_hostname.empty())
    {
        return "";
    }

    String address;
    if (forceAddress)
    {
        constexpr int maxBufferSize = 128;
        array<char, maxBufferSize> buffer {};

        const void* addr {nullptr};
        // Get the pointer to the address itself, different fields in IPv4 and IPv6
        if (any().sa_family == AF_INET)
        {
            addr = (void*) &(ip_v4().sin_addr);
        }
        else
        {
            addr = (void*) &(ip_v6().sin6_addr);
        }

        if (inet_ntop(any().sa_family, addr, buffer.data(), sizeof(buffer) - 1) == nullptr)
        {
            throw SystemException("Can't print IP address");
        }

        address = String(buffer.data());
    }
    else
    {
        address = m_hostname;
    }

    if (any().sa_family == AF_INET6 && m_hostname.find(':') != std::string::npos)
    {
        str << "[" << address << "]:" << m_port;
    }
    else
    {
        str << address << ":" << m_port;
    }

    return str.str();
}
