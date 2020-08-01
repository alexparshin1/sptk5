/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#include <sptk5/net/BaseSocket.h>
#include <sptk5/Printer.h>
#include <sptk5/RegularExpression.h>
#include <sptk5/SystemException.h>

#ifndef _WIN32
#include <netinet/in.h>
#include <netdb.h>

#include <utility>
#include <sptk5/net/Host.h>

#endif

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
: m_hostname(hostname), m_port(port)
{
    getHostAddress();
    setPort(m_port);
}

Host::Host(const String& hostAndPort)
{
    RegularExpression matchHost(R"(^(\[.*\]|[^\[\]:]*)(:\d+)?)");
    auto matches = matchHost.m(hostAndPort);
    if (matches) {
        m_hostname = matches[size_t(0)].value;
        if (matches.groups().size() > 1)
            m_port = (uint16_t) string2int(matches[1].value.substr(1));
        getHostAddress();
        setPort(m_port);
    } else {
        memset(&m_address, 0, sizeof(m_address));
    }
}

Host::Host(const sockaddr_in* addressAndPort)
{
    socklen_t addressLen = 0;
    const sockaddr_in6* addressAndPort6;

    switch (addressAndPort->sin_family) {
        case AF_INET:
            addressLen = sizeof(sockaddr_in);
            memcpy(m_address, addressAndPort, addressLen);
            m_port = htons(ip_v4().sin_port);
            break;
        case AF_INET6:
            addressAndPort6 = (const sockaddr_in6*) addressAndPort;
            addressLen = sizeof(sockaddr_in6);
            memcpy(m_address, addressAndPort6, addressLen);
            m_port = htons(ip_v6().sin6_port);
            break;
        default:
            break;
    }

#ifdef _WIN32
    char hbuf[NI_MAXHOST];
    char sbuf[NI_MAXSERV];
    if (getnameinfo((const sockaddr*)m_address, addressLen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), 0) == 0)
        m_hostname = hbuf;
#else
    char hbuf[NI_MAXHOST];
    char sbuf[NI_MAXSERV];
    if (getnameinfo((const sockaddr*) m_address, addressLen, hbuf, sizeof(hbuf), sbuf, sizeof(sbuf), 0) == 0)
        m_hostname = hbuf;
#endif
}

Host::Host(const Host& other)
: m_hostname(other.m_hostname), m_port(other.m_port)
{
    SharedLock(other.m_mutex);
    memcpy(&m_address, &other.m_address, sizeof(m_address));
}

Host::Host(Host&& other) noexcept
: m_hostname(exchange(other.m_hostname,"")), m_port(exchange(other.m_port,0))
{
    SharedLock(other.m_mutex);
    memcpy(&m_address, &other.m_address, sizeof(m_address));
}

Host& Host::operator = (const Host& other)
{
    if (&other != this) {
        SharedLockInt lock1(other.m_mutex);
        UniqueLockInt lock2(m_mutex);
        m_hostname = other.m_hostname;
        m_port = other.m_port;
        memcpy(&m_address, &other.m_address, sizeof(m_address));
    }
    return *this;
}

Host& Host::operator = (Host&& other) noexcept
{
    CopyLock(m_mutex, other.m_mutex);
    m_hostname = other.m_hostname;
    m_port = other.m_port;
    memcpy(&m_address, &other.m_address, sizeof(m_address));
    return *this;
}

bool Host::operator == (const Host& other) const
{
    CompareLock(m_mutex, other.m_mutex);
    return toString(true) == other.toString(true);
}

bool Host::operator != (const Host& other) const
{
    CompareLock(m_mutex, other.m_mutex);
    return toString(true) != other.toString(true);
}

void Host::setPort(uint16_t p)
{
    UniqueLock(m_mutex);
    m_port = p;
    switch (any().sa_family) {
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


static struct addrinfo* safeGetAddrInfo(const String& hostname)
{
    static SharedMutex getaddrinfoMutex;

    struct addrinfo hints = {};
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;          // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // Socket type
    hints.ai_protocol = 0;

    UniqueLock(getaddrinfoMutex);

    struct addrinfo* result;
    int rc = getaddrinfo(hostname.c_str(), nullptr, &hints, &result);
    if (rc != 0)
        throw Exception(gai_strerror(rc));

    return result;
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

    switch (any().sa_family) {
    case AF_INET:
        memcpy(&ip_v4().sin_addr, host_info->h_addr, size_t(host_info->h_length));
        break;
    case AF_INET6:
        memcpy(&ip_v6().sin6_addr, host_info->h_addr, size_t(host_info->h_length));
        break;
    }
#else
    struct addrinfo* result = safeGetAddrInfo(m_hostname);

    UniqueLock(m_mutex);
    memset(&m_address, 0, sizeof(m_address));
    memcpy(&m_address, (struct sockaddr_in*) result->ai_addr, result->ai_addrlen);

    freeaddrinfo(result);
#endif
}

String Host::toString(bool forceAddress) const
{
    SharedLock(m_mutex);
    std::stringstream str;

    if (m_hostname.empty())
        return "";

    String address;
    if (forceAddress) {
        char buffer[128];

        const void *addr;
        // Get the pointer to the address itself, different fields in IPv4 and IPv6
        if (any().sa_family == AF_INET) {
            addr = (void*) &(ip_v4().sin_addr);
        } else {
            addr = (void*) &(ip_v6().sin6_addr);
        }

        if (inet_ntop(any().sa_family, addr, buffer, sizeof(buffer) - 1) == nullptr)
            throw SystemException("Can't print IP address");

        address = buffer;
    } else
        address = m_hostname;

    if (any().sa_family == AF_INET6 && m_hostname.find(':') != std::string::npos)
        str << "[" << address << "]:" << m_port;
    else
        str << address << ":" << m_port;

    return str.str();
}


#if USE_GTEST

const String testHost("www.google.com:80");

TEST(SPTK_Host, ctorHostname)
{
    Host google1(testHost);
    EXPECT_STREQ(testHost.c_str(), google1.toString(false).c_str());
    EXPECT_STREQ("www.google.com", google1.hostname().c_str());
    EXPECT_EQ(80, google1.port());

    Host google(google1.toString(true));
    EXPECT_TRUE(google1 == google);
}

TEST(SPTK_Host, ctorAddress)
{
    Host host("11.22.33.44", 22);
    EXPECT_STREQ("11.22.33.44", host.hostname().c_str());
    EXPECT_EQ(22, host.port());
}

TEST(SPTK_Host, ctorAddressStruct)
{
    String testHostAndPort { "bitbucket.com:80" };
    Host host1(testHostAndPort);

    sockaddr_in address;
    host1.getAddress(address);
    Host host2(&address);

    EXPECT_STREQ(host1.toString(true).c_str(), host2.toString(true).c_str());
    EXPECT_STREQ(testHostAndPort.c_str(), host2.toString(false).c_str());
    EXPECT_EQ(host1.port(), host2.port());
}

TEST(SPTK_Host, ctorCopy)
{
    Host host1("11.22.33.44", 22);
    Host host2(host1);
    EXPECT_STREQ("11.22.33.44", host2.hostname().c_str());
    EXPECT_EQ(22, host2.port());
}

TEST(SPTK_Host, ctorMove)
{
    Host host1("11.22.33.44", 22);
    Host host2(move(host1));
    EXPECT_STREQ("", host1.hostname().c_str());
    EXPECT_EQ(0, host1.port());
    EXPECT_STREQ("11.22.33.44", host2.hostname().c_str());
    EXPECT_EQ(22, host2.port());
}

TEST(SPTK_Host, assign)
{
    Host host1("11.22.33.44", 22);
    Host host2 = host1;
    EXPECT_STREQ("11.22.33.44", host2.hostname().c_str());
    EXPECT_EQ(22, host2.port());
}

TEST(SPTK_Host, move)
{
    Host host1("11.22.33.44", 22);
    Host host2 = move(host1);
    EXPECT_STREQ("", host1.hostname().c_str());
    EXPECT_EQ(0, host1.port());
    EXPECT_STREQ("11.22.33.44", host2.hostname().c_str());
    EXPECT_EQ(22, host2.port());
}

TEST(SPTK_Host, compare)
{
    Host host1("11.22.33.44", 22);
    Host host2(host1);
    Host host3("11.22.33.45", 22);
    Host host4("11.22.33.44", 23);

    EXPECT_TRUE(host1 == host2);
    EXPECT_FALSE(host1 != host2);

    EXPECT_FALSE(host1 == host3);
    EXPECT_TRUE(host1 != host3);

    EXPECT_FALSE(host1 == host4);
    EXPECT_TRUE(host1 != host4);
}

#endif
