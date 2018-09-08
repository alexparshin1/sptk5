/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       cnet - description                                     ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday September 13 2017                            ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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
#include <sptk5/net/BaseSocket.h>
#include <sptk5/SystemException.h>

using namespace std;
using namespace sptk;

Host::Host()
: m_port(0)
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
: m_port(0)
{
    RegularExpression matchHost("^(\\[.*\\]|[^\\[\\]:]*)(:\\d+)?");
    Strings matches;
    if (matchHost.m(hostAndPort, matches)) {
        m_hostname = matches[0];
        if (matches.size() > 1)
            m_port = (uint16_t) string2int(matches[1].substr(1));
        getHostAddress();
		setPort(m_port);
	} else {
        memset(&m_address, 0, sizeof(m_address));
    }
}

Host::Host(const Host& other)
: m_hostname(other.m_hostname), m_port(other.m_port)
{
    lock_guard<mutex> lock(other.m_mutex);
    memcpy(&m_address, &other.m_address, sizeof(m_address));
}

Host::Host(Host&& other) noexcept
: m_hostname(move(other.m_hostname)), m_port(other.m_port)
{
    lock_guard<mutex> lock(other.m_mutex);
    memcpy(&m_address, &other.m_address, sizeof(m_address));
}

Host& Host::operator = (const Host& other)
{
    lock_guard<mutex> lock1(other.m_mutex);
    lock_guard<mutex> lock2(m_mutex);
    m_hostname = other.m_hostname;
    m_port = other.m_port;
    memcpy(&m_address, &other.m_address, sizeof(m_address));
    return *this;
}

Host& Host::operator = (Host&& other) noexcept
{
    lock_guard<mutex> lock1(other.m_mutex);
    lock_guard<mutex> lock2(m_mutex);
    m_hostname = other.m_hostname;
    m_port = other.m_port;
    memcpy(&m_address, &other.m_address, sizeof(m_address));
    return *this;
}

bool Host::operator == (const Host& other) const
{
    return toString(true) == other.toString(true);
}

bool Host::operator != (const Host& other) const
{
    return toString(true) != other.toString(true);
}

void Host::setPort(uint16_t p)
{
	m_port = p;
	switch (m_address.any.sa_family) {
	case AF_INET:
		m_address.ip_v4.sin_port = htons(uint16_t(m_port));
		break;
	case AF_INET6:
		m_address.ip_v6.sin6_port = htons(uint16_t(m_port));
		break;
	}
}

void Host::getHostAddress()
{
    static mutex getaddrinfoMutex;

    memset(&m_address, 0, sizeof(m_address));

#ifdef _WIN32
    struct hostent* host_info = gethostbyname(m_hostname.c_str());
	if (host_info == nullptr)
		BaseSocket::throwSocketError("Can't get host info for " + m_hostname, __FILE__, __LINE__);

	m_address.any.sa_family = host_info->h_addrtype;

	switch (m_address.any.sa_family) {
	case AF_INET:
		memcpy(&m_address.ip_v4.sin_addr, host_info->h_addr, size_t(host_info->h_length));
		break;
	case AF_INET6:
		memcpy(&m_address.ip_v6.sin6_addr, host_info->h_addr, size_t(host_info->h_length));
		break;
	}

#else
    struct addrinfo hints = {};
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;          // IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM;    // Socket type
    //hints.ai_flags = AI_PASSIVE;      /* For wildcard IP address */
    hints.ai_protocol = 0;

    lock_guard<mutex> lock(getaddrinfoMutex);

    struct addrinfo* result;
    int rc = getaddrinfo(m_hostname.c_str(), nullptr, &hints, &result);
    if (rc != 0)
        throw Exception(gai_strerror(rc));

    memcpy(&m_address, (struct sockaddr_in*) result->ai_addr, result->ai_addrlen);

    freeaddrinfo(result);
#endif
}

String Host::toString(bool forceAddress) const
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::stringstream str;

	if (m_hostname.empty())
		return "";

    string address;
    if (forceAddress) {
        char buffer[128];
#ifdef _WIN32
		void *addr;
		// Get the pointer to the address itself, different fields in IPv4 and IPv6
		if (m_address.any.sa_family == AF_INET) {
			struct sockaddr_in *ipv4 = (struct sockaddr_in *) &m_address;
			addr = &(ipv4->sin_addr);
		} else {
			struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *) &m_address;
			addr = &(ipv6->sin6_addr);
		}
		if (inet_ntop(m_address.any.sa_family, addr, buffer, sizeof(buffer) - 1) == nullptr)
#else
		if (inet_ntop(m_address.any.sa_family, &m_address, buffer, sizeof(buffer) - 1) == nullptr)
#endif
            throw SystemException("Can't print IP address");
        address = buffer;
    } else {
        address = m_hostname;
    }

    if (m_address.any.sa_family == AF_INET6 && m_hostname.find(':') != std::string::npos)
        str << "[" << address << "]:" << m_port;
    else
        str << address << ":" << m_port;

    return str.str();
}


#if USE_GTEST
#include <gtest/gtest.h>

const char* testHost = "www.google.com:80";

TEST(SPTK_Host, ctorHostname)
{
    Host google1(testHost);
    EXPECT_STREQ(testHost, google1.toString(false).c_str());
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

#endif
