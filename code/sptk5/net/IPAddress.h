/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       IPAddress.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday August 16, 2017                              ║
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

#ifndef __IPADDRESS_H__
#define __IPADDRESS_H__

#include <sptk5/sptk.h>

#ifndef _WIN32
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sptk5/Strings.h>

/**
 * A socket handle is an integer
 */
typedef int SOCKET;
typedef sa_family_t SOCKET_ADDRESS_FAMILY;

#ifdef __APPLE__
typedef int socklen_t;
#endif

/**
 * A value to indicate an invalid handle
 */
#define INVALID_SOCKET -1

#else
#include <winsock2.h>
    #include <windows.h>
    typedef int socklen_t;
    typedef unsigned short SOCKET_ADDRESS_FAMILY;
#endif

namespace sptk
{

/**
 * @addtogroup network Network Classes
 * @{
 */

/**
 * @brief IPv4 and IPv6 address presentation
 */
class SP_EXPORT IPAddress
{
    /**
     * Shared storage for IPv4 and IPv6 addresses
     */
    union {
        sockaddr_in     ipv4;
        sockaddr_in6    ipv6;
        sockaddr        generic;
    } m_address;

    String  m_addressStr;
public:

    /**
     * @brief Default constructor
     */
    IPAddress();

    /**
     * @brief Constructor
     * @param address const sockaddr&, IPv4 address
     */
    explicit IPAddress(const sockaddr& address);

    /**
     * @brief Copy constructor
     * @param other const IPAddress&, Other address
     */
    IPAddress(const IPAddress& other);

    /**
     * @brief Assignment
     * @param other const IPAddress&, Other address
     */
    IPAddress& operator=(const IPAddress& other);

    /**
     * @brief Get address data
     */
    const sockaddr* address() const
    {
        return &m_address.generic;
    }

    /**
     * Return length of address
     * @return length of address
     */
    size_t length() const
    {
        return addressLength(m_address.generic);
    }

    /**
     * Return IP address as a string
     * @return string presentation of IP address
     */
    const String& toString() const
    {
        return m_addressStr;
    }

    /**
     * Return length of actual address
     * @param address const sockaddr&, Address data
     * @return length of actual address
     */
    static size_t addressLength(const sockaddr& address)
    {
        if (address.sa_family == AF_INET)
            return sizeof(sockaddr_in);
        return sizeof(sockaddr_in6);
    }
};

/**
 * @}
 */
}
#endif
