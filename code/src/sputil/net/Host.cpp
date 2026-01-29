/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/net/Socket.h>

#include <utility>

using namespace std;
using namespace sptk;

namespace {
void checkSocketsInitialized()
{
#ifdef _WIN32
    static mutex initMutex;
    static bool  initialized = false;
    scoped_lock  lock(initMutex);
    if (!initialized)
    {
        Socket::init();
        initialized = true;
    }
#endif
}
} // namespace

Host::Host() noexcept
{
    checkSocketsInitialized();
    memset(&m_address, 0, sizeof(m_address));
}

Host::Host(String hostname, const uint16_t port)
    : m_hostname(std::move(hostname))
    , m_port(port)
{
    checkSocketsInitialized();
    getHostAddress();
    setPort(m_port);
}

Host::Host(const String& hostAndPort)
{
    static const RegularExpression matchHostNameOrIpv4(R"(^([\w\d\-\.]+)(:\d{2,5})?$)");
    static const RegularExpression matchIpv6(R"(^\[([\w\d\-\.:]+)\](:\d{2,5})?$)");

    checkSocketsInitialized();
    const auto matches = hostAndPort.startsWith("[")
                             ? matchIpv6.m(hostAndPort)
                             : matchHostNameOrIpv4.m(hostAndPort);

    if (matches)
    {
        m_hostname = matches[0].value;

        if (matches.groups().size() > 1)
        {
            m_port = static_cast<uint16_t>(string2int(matches[1].value.substr(1)));
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
    checkSocketsInitialized();
    constexpr socklen_t addressLen = sizeof(sockaddr_in);
    memcpy(m_address.data(), addressAndPort, addressLen);
    m_port = ntohs(ip_v4().sin_port);

    setHostNameFromAddress(addressLen);
}

Host::Host(const sockaddr_in6* addressAndPort)
{
    checkSocketsInitialized();
    constexpr socklen_t addressLen = sizeof(sockaddr_in6);

    const auto* addressAndPort6 = addressAndPort;
    memcpy(bit_cast<sockaddr_in6*>(m_address.data()), addressAndPort6, addressLen);
    m_port = ntohs(ip_v6().sin6_port);

    setHostNameFromAddress(addressLen);
}

void Host::setHostNameFromAddress(const socklen_t addressLen)
{
    array<char, NI_MAXHOST> hostBuffer {};
    array<char, NI_MAXSERV> addressBuffer {};

    if (getnameinfo(bit_cast<const sockaddr*>(m_address.data()), addressLen, hostBuffer.data(), sizeof(hostBuffer), addressBuffer.data(),
                    sizeof(addressBuffer), 0) == 0)
    {
        m_hostname = String(hostBuffer.data());
    }
    else
    {
        m_hostname = ipAddressToString(m_address.data());
    }
}

Host::Host(const Host& other)
{
    const scoped_lock lock(other.m_mutex);
    m_hostname = other.m_hostname;
    m_port = other.m_port;
    memcpy(&m_address, &other.m_address, sizeof(m_address));
}

Host::Host(Host&& other) noexcept
    : m_hostname(exchange(other.m_hostname, ""))
    , m_port(other.m_port)
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
    try
    {
        return toString(true) == other.toString(true);
    }
    catch (Exception&)
    {
        return false;
    }
}

void Host::setPort(const uint16_t port)
{
    const scoped_lock lock(m_mutex);
    m_port = port;
    switch (any().sa_family)
    {
        case AF_INET:
            ip_v4().sin_port = htons(m_port);
            break;
        case AF_INET6:
            ip_v6().sin6_port = htons(m_port);
            break;
        default:
            break;
    }
}

void Host::getHostAddress()
{
    const scoped_lock lock(m_mutex);

    addrinfo hints = {};
    memset(&hints, 0, sizeof(addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = 0;

    addrinfo* result = nullptr;
    if (const int exitCode = getaddrinfo(m_hostname.c_str(), nullptr, &hints, &result);
        exitCode != 0)
    {
        throw Exception(gai_strerror(exitCode));
    }

    memset(&m_address, 0, sizeof(m_address));
    memcpy(&m_address, bit_cast<sockaddr_in*>(result->ai_addr), result->ai_addrlen);

    freeaddrinfo(result);
}

String Host::ipAddressToString(const uint8_t* addr) const
{
    constexpr int              maxBufferSize = 128;
    array<char, maxBufferSize> buffer {};

    if (inet_ntop(any().sa_family, addr, buffer.data(), sizeof(buffer) - 1) == nullptr)
    {
        throw SystemException("Can't print IP address");
    }

    return {buffer.data()};
}

String Host::toString(const bool forceAddress) const
{
    const scoped_lock lock(m_mutex);
    String            str;

    if (!m_hostname.empty())
    {
        String address;
        if (forceAddress)
        {
            const uint8_t* addr;
            // Get the pointer to the address itself, different fields in IPv4 and IPv6
            if (any().sa_family == AF_INET)
            {
                addr = bit_cast<uint8_t*>(&(ip_v4().sin_addr));
            }
            else
            {
                addr = bit_cast<uint8_t*>(&(ip_v6().sin6_addr));
            }

            address = ipAddressToString(addr);
        }
        else
        {
            address = m_hostname;
        }

        if (any().sa_family == AF_INET6)
        {
            str = format("[{}]:{}", address.c_str(), m_port);
        }
        else
        {
            str = format("{}:{}", address.c_str(), m_port);
        }
    }

    return str;
}
