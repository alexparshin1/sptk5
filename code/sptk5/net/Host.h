/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#include <cstring>
#include <mutex>
#include <sptk5/Strings.h>
#include <sstream>

#ifndef _WIN32

#include <netinet/in.h>

#else
#include <WS2tcpip.h>
#include <winsock2.h>
#endif

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * Network host information
 */
class SP_EXPORT Host
{
    mutable std::mutex                        m_mutex;      ///< Mutex to protect internal class data
    String                                    m_hostname;   ///< Host name or IP address
    uint16_t                                  m_port {0};   ///< Port number
    std::array<uint8_t, sizeof(sockaddr_in6)> m_address {}; ///< Storage for IPv4 and IPv6 addresses

    /**
     * Get address presentation as generic IP address
     * @return address presentation as generic IP address
     */
    sockaddr& any()
    {
        return *(sockaddr*) m_address.data();
    }

    /**
     * Get address presentation as generic IP address (read-only)
     * @return address presentation as generic IP address
     */
    const sockaddr& any() const
    {
        return *(const sockaddr*) m_address.data();
    }

    /**
     * Get address presentation as IPv4 address
     * @return address presentation as IPv4 address
     */
    sockaddr_in& ip_v4()
    {
        return *(sockaddr_in*) m_address.data();
    }

    /**
     * Get address presentation as IPv4 address (read-only)
     * @return address presentation as IPv4 address
     */
    const sockaddr_in& ip_v4() const
    {
        return *(const sockaddr_in*) m_address.data();
    }

    /**
     * Get address presentation as IPv6 address
     * @return address presentation as IPv6 address
     */
    sockaddr_in6& ip_v6()
    {
        return *(sockaddr_in6*) m_address.data();
    }

    /**
     * Get address presentation as IPv6 address (read-only)
     * @return address presentation as IPv6 address
     */
    const sockaddr_in6& ip_v6() const
    {
        return *(const sockaddr_in6*) m_address.data();
    }

    /**
     * Get host address
     */
    void getHostAddress();

    /**
     * Set port number
     * @param port                 Port number
     */
    void setPort(uint16_t port);

public:
    /**
     * Default constructor
     */
    Host() noexcept;

    /**
     * Constructor
     * @param hostname          Host name or IP address
     * @param port              Port number
     */
    Host(String hostname, uint16_t port);

    /**
     * Constructor
     * In order to work with IPv6 address, enclose address part in square brackets.
     * @param hostAndPort       The host and port definition, in the format ipv4addr:port.
     */
    explicit Host(const String& hostAndPort);

    /**
     * Constructor
     * @param addressAndPort    The host and port, IPv4
     */
    explicit Host(const sockaddr_in* addressAndPort);

    /**
     * Constructor
     * @param addressAndPort    The host and port, IPv6
     */
    explicit Host(const sockaddr_in6* addressAndPort);

    /**
     * Copy constructor
     * @param other             The other object
     */
    Host(const Host& other);

    /**
     * Move constructor
     * @param other             The other object
     */
    Host(Host&& other) noexcept;

    /**
     * Destructor
     */
    virtual ~Host() = default;

    /**
     * Assign from another host
     * @param other             The other object
     */
    Host& operator=(const Host& other);

    /**
     * Move assignment from another host
     * @param other             The other object
     */
    Host& operator=(Host&& other) noexcept;

    /**
     * Compare to another host
     * @param other             The other object
     * @return true if objects have equal data
     */
    bool operator==(const Host& other) const;

    /**
     * Get host name
     * @return host name
     */
    const String& hostname() const
    {
        std::scoped_lock lock(m_mutex);
        return m_hostname;
    }

    /**
     * Set port number
     * @param p                 Port number
     */
    void port(uint16_t p)
    {
        setPort(p);
    }

    /**
     * Get port number
     * @return port number
     */
    uint16_t port() const
    {
        std::scoped_lock lock(m_mutex);
        return m_port;
    }

    /**
     * Get host name and port as a string.
     * IPv6 addresses are enclosed in square brackets.
     * @param forceAddress      If true then use IP address instead of hostname
     * @return host name and port string
     */
    String toString(bool forceAddress = false) const;

    /**
     * Get host address
     */
    void getAddress(sockaddr_in& address) const
    {
        std::scoped_lock lock(m_mutex);
        memcpy(&address, &m_address, sizeof(address));
    }

    /**
     * Get host address
     */
    void getAddress(sockaddr_in6& address) const
    {
        std::scoped_lock lock(m_mutex);
        memcpy(&address, &m_address, sizeof(address));
    }

    void setHostNameFromAddress(socklen_t addressLen);
};

using SHost = std::shared_ptr<Host>;

/**
 * @brief Case-insensitive host compare class.
 *
 * Lower case host compare class is really useful if we need
 * a case-independent host map
 */
class SP_EXPORT HostCompare
{
public:
    /**
     * @brief Compare method
     * @param s1            First host
     * @param s2            Second host
     */
    bool operator()(const Host& s1, const Host& s2) const
    {
#ifdef WIN32
        return stricmp(s1.toString(true).c_str(), s2.toString(true).c_str()) > 0;
#else
        return strcasecmp(s1.toString(true).c_str(), s2.toString(true).c_str()) > 0;
#endif
    }
};


/**
 * @}
 */

} // namespace sptk
