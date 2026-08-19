/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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

#pragma once

#include <sptk5/sptk.h>

#ifndef _WIN32
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sptk5/Strings.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

/**
 * @brief A socket handle is an integer.
 */
using SOCKET = int;
using SOCKET_ADDRESS_FAMILY = sa_family_t;

#ifdef __APPLE__
using socklen_t = int;
#endif

/**
 * @brief A value to indicate an invalid handle.
 */
#define INVALID_SOCKET -1

#else
#include <winsock2.h>

#include <windows.h>
using socklen_t = int;
using SOCKET_ADDRESS_FAMILY = unsigned short;
#endif

namespace sptk {
/**
 * @addtogroup network Network Classes.
 * @{
 */

/**
 * @brief IPv4 and IPv6 address presentation.
 */
class SP_EXPORT IPAddress
{
    /**
     * @brief Shared storage for IPv4 and IPv6 addresses.
     */
    union
    {
        sockaddr_in  ipv4;
        sockaddr_in6 ipv6;
        sockaddr     generic;
    } m_address;

    String m_addressStr;

public:
    /**
     * @brief Default constructor.
     */
    IPAddress();

    /**
     * @brief Constructor.
     * @param address IPv4 address.
     */
    explicit IPAddress(const sockaddr& address);

    /**
     * @brief Copy constructor.
     * @param other another address.
     */
    IPAddress(const IPAddress& other);

    /**
     * @brief Assignment.
     * @param other Another address.
     */
    IPAddress& operator=(const IPAddress& other);

    /**
     * @brief Get address data.
     */
    const sockaddr* address() const
    {
        return &m_address.generic;
    }

    /**
     * @brief Return length of address.
     * @return length of address.
     */
    size_t length() const
    {
        return addressLength(m_address.generic);
    }

    /**
     * @brief Return the IP address as a string.
     * @return string presentation of IP address.
     */
    const String& toString() const
    {
        return m_addressStr;
    }

    /**
     * @brief Return length of actual address.
     * @param address Address data.
     * @return length of actual address.
     */
    static size_t addressLength(const sockaddr& address)
    {
        if (address.sa_family == AF_INET)
        {
            return sizeof(sockaddr_in);
        }
        return sizeof(sockaddr_in6);
    }
};

/**
 * @}
 */
} // namespace sptk
