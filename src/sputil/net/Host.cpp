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

using namespace std;
using namespace sptk;

Host::Host() 
: m_port(0)
{
    memset(&m_address, 0, sizeof(m_address));
}

Host::Host(const string& hostname, uint16_t port)
: m_hostname(hostname), m_port(port)
{
    getHostAddress();
    m_address.sin_port = htons(uint16_t(m_port));
}

Host::Host(const string& hostAndPort)
: m_port(0)
{
    RegularExpression matchHost("^(\\[.*\\]|[^\\[].*)(:\\d+)?");
    Strings matches;
    if (matchHost.m(hostAndPort, matches)) {
        m_hostname = matches[0];
        if (matches.size() > 1)
            m_port = (uint16_t) string2int(matches[1]);
        getHostAddress();
        m_address.sin_port = htons(uint16_t(m_port));
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
    m_hostname = move(other.m_hostname);
    m_port = other.m_port;
    memcpy(&m_address, &other.m_address, sizeof(m_address));
    return *this;
}

bool Host::operator == (const Host& other) const
{
    lock_guard<mutex> lock1(other.m_mutex);
    lock_guard<mutex> lock2(m_mutex);
    return m_hostname == other.m_hostname && m_port == other.m_port;
}

bool Host::operator != (const Host& other) const
{
    lock_guard<mutex> lock1(other.m_mutex);
    lock_guard<mutex> lock2(m_mutex);
    return m_hostname != other.m_hostname || m_port != other.m_port;
}

void Host::getHostAddress()
{
    static mutex getaddrinfoMutex;

    memset(&m_address, 0, sizeof(m_address));

#ifdef _WIN32
    struct hostent* host_info = gethostbyname(m_hostname.c_str());
	if (host_info == nullptr)
		BaseSocket::throwSocketError("Can't get host info for " + m_hostname, __FILE__, __LINE__);
	m_address.sin_family = host_info->h_addrtype;
    memcpy(&m_address.sin_addr, host_info->h_addr, size_t(host_info->h_length));
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
