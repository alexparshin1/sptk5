/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       IPAddress.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday August 16, 2017                              ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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
    union {
        sockaddr_in     ipv4;
        sockaddr_in6    ipv6;
        sockaddr        generic;
    } m_address;
public:

    /**
     * @brief Default constructor
     */
    IPAddress()
    {
        memset(&m_address, 0, sizeof(m_address));
    }

    /**
     * @brief Constructor
     * @param address const sockaddr_in*, IPv4 address
     */
    IPAddress(const sockaddr_in* address)
    {
        memcpy(&m_address, address, sizeof(sockaddr_in));
    }

    /**
     * @brief Constructor
     * @param address const sockaddr_in6*, IPv6 address
     */
    IPAddress(const sockaddr_in6* address)
    {
        memcpy(&m_address, address, sizeof(sockaddr_in6));
    }

    /**
     * @brief Copy constructor
     * @param other const IPAddress&, Other address
     */
    IPAddress(const IPAddress& address);

    /**
     * @brief Assignment
     * @param other const IPAddress&, Other address
     */
    IPAddress& operator=(const IPAddress& address)
    {
        memcpy(&m_address, address, sizeof(m_address));
    }

    /**
     * @brief Get address data
     */
    const sockaddr* address() const
    {
        return &m_address.generic;
    }
};

/**
 * @}
 */
}
#endif
