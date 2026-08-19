/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-04-21                                             ║
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

#include <sptk5/RegularExpression.h>
#include <sptk5/SystemException.h>
#include <sptk5/net/Socket.h>

#include <format>
#include <utility>

using namespace std;
using namespace sptk;

const RegularExpression Host::m_matchHostNameOrIpv4(R"(^([\w\d\-\.]+)(:\d{1,5})?$)");
const RegularExpression Host::m_matchIpv6(R"(^\[([\w\d\-\.:]+)\](:\d{1,5})?$)");

#ifdef _WIN32
namespace {
void checkSocketsInitialized()
{
    static atomic_bool initialized = false;
    static mutex       initMutex;

    if (!initialized.load())
    {
        scoped_lock lock(initMutex);
        if (!initialized.load())
        {
            Socket::init();
            initialized = true;
        }
    }
}
} // namespace
#endif

Host::Host() noexcept
{
#ifdef _WIN32
    checkSocketsInitialized();
#endif
}

Host::Host(string hostname, const uint16_t port)
    : m_hostname(std::move(hostname))
    , m_port(port)
{
#ifdef _WIN32
    checkSocketsInitialized();
#endif
    getHostAddressUnlocked();
    setPort(m_port);
}

Host::Host(const string& hostAndPort)
{
#ifdef _WIN32
    checkSocketsInitialized();
#endif
    const auto matches = hostAndPort.starts_with("[")
                             ? m_matchIpv6.m(hostAndPort)
                             : m_matchHostNameOrIpv4.m(hostAndPort);

    if (!matches)
    {
        throw Exception("Can't parse host and port: " + hostAndPort);
    }

    m_hostname = matches[0].value;

    if (matches.groups().size() > 1)
    {
        if (const auto portStr = matches[1].value;
            !portStr.empty())
        {
            auto port = stoi(portStr.substr(1));
            if (port < 0 || port > 65535)
            {
                throw Exception("Invalid port number: " + portStr);
            }
            m_port = static_cast<uint16_t>(port);
        }
    }
    getHostAddressUnlocked();
    setPort(m_port);
}

Host::Host(const sockaddr_in* addressAndPort)
{
#ifdef _WIN32
    checkSocketsInitialized();
#endif
    constexpr socklen_t addressLen = sizeof(sockaddr_in);
    memcpy(m_address.data(), addressAndPort, addressLen);
    m_port = ntohs(ip_v4().sin_port);

    setHostNameFromAddress(addressLen);
}

Host::Host(const sockaddr_in6* addressAndPort)
{
#ifdef _WIN32
    checkSocketsInitialized();
#endif
    constexpr socklen_t addressLen = sizeof(sockaddr_in6);

    const auto* addressAndPort6 = addressAndPort;
    memcpy(m_address.data(), addressAndPort6, addressLen);
    m_port = ntohs(ip_v6().sin6_port);

    setHostNameFromAddress(addressLen);
}

void Host::setHostNameFromAddress(const socklen_t addressLen)
{
    array<char, NI_MAXHOST> hostBuffer {};
    array<char, NI_MAXSERV> addressBuffer {};

    if (getnameinfo(reinterpret_cast<const sockaddr*>(m_address.data()), addressLen, hostBuffer.data(), sizeof(hostBuffer), addressBuffer.data(),
                    sizeof(addressBuffer), 0) == 0)
    {
        m_hostname = string(hostBuffer.data());
    }
    else
    {
        const uint8_t* addr;
        // Get the pointer to the address itself, different fields in IPv4 and IPv6
        if (any().sa_family == AF_INET)
        {
            addr = reinterpret_cast<uint8_t*>(&(ip_v4().sin_addr));
        }
        else
        {
            addr = reinterpret_cast<uint8_t*>(&(ip_v6().sin6_addr));
        }

        m_hostname = ipAddressToString(addr);
    }
}

Host::Host(const Host& other)
{
    m_hostname = other.m_hostname;
    m_port = other.m_port;
    m_address = other.m_address;
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

#ifdef _WIN32
constexpr int EAI_ADDRFAMILY = EAI_FAMILY;
#endif

void Host::getHostAddressUnlocked()
{
    addrinfo* result = nullptr;
    string    error;

    constexpr addrinfo hintsInet = {.ai_flags = 0, .ai_family = AF_INET, .ai_socktype = SOCK_STREAM, .ai_protocol = 0};
    auto               exitCode = getaddrinfo(m_hostname.c_str(), nullptr, &hintsInet, &result);
    if (exitCode != 0)
    {
        error = gai_strerror(exitCode);
    }

    if (exitCode == EAI_ADDRFAMILY || exitCode == EAI_NONAME)
    {
        constexpr addrinfo hintsInet6 = {.ai_flags = 0, .ai_family = AF_INET6, .ai_socktype = SOCK_STREAM, .ai_protocol = 0};
        exitCode = getaddrinfo(m_hostname.c_str(), nullptr, &hintsInet6, &result);
        if (exitCode != 0)
        {
            error = gai_strerror(exitCode);
        }
    }

    if (exitCode == 0)
    {
        memcpy(&m_address, result->ai_addr, result->ai_addrlen);
        freeaddrinfo(result);
    }
    else
    {
        throw Exception(format("Can't resolve hostname: {}. Error: {}.", m_hostname, error));
    }
}

string Host::ipAddressToString(const uint8_t* addr) const
{
    constexpr auto             maxBufferSize = 128;
    array<char, maxBufferSize> buffer {};

    if (inet_ntop(any().sa_family, addr, buffer.data(), sizeof(buffer) - 1) == nullptr)
    {
        throw SystemException("Can't print IP address");
    }

    return {buffer.data()};
}

string Host::toString(const bool forceAddress) const
{
    string str;

    if (!m_hostname.empty())
    {
        string address;
        if (forceAddress)
        {
            const uint8_t* addr;
            // Get the pointer to the address itself, different fields in IPv4 and IPv6
            if (any().sa_family == AF_INET)
            {
                addr = reinterpret_cast<const uint8_t*>(&(ip_v4().sin_addr));
            }
            else
            {
                addr = reinterpret_cast<const uint8_t*>(&(ip_v6().sin6_addr));
            }

            address = ipAddressToString(addr);
        }
        else
        {
            address = m_hostname;
        }

        if (any().sa_family == AF_INET6)
        {
            str = format("[{}]:{}", address, m_port);
        }
        else
        {
            str = format("{}:{}", address, m_port);
        }
    }

    return str;
}
