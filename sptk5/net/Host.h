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

#ifndef __HOST_H__
#define __HOST_H__

#include <sptk5/Strings.h>
#include <cstring>
#include <mutex>
#include <sstream>

#ifndef _WIN32

#include <netinet/in.h>

#else
#include <winsock2.h>
#include <WS2tcpip.h>
#endif

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * Network host information
 */
class Host
{
    mutable std::mutex  m_mutex;        ///< Mutex to protect internal class data
    String              m_hostname;     ///< Host name or IP address
    uint16_t            m_port;         ///< Port number
	union {
		struct sockaddr	 any;
		struct sockaddr_in  ip_v4;
		struct sockaddr_in6 ip_v6;
	}					m_address;      ///< Host address

    /**
     * Get host address
     */
    void getHostAddress();

	/**
	 * Set port number
	 * @param p                 Port number
	 */
	void setPort(uint16_t p);

public:

    /**
     * Default constructor
     */
    Host();

    /**
     * Constructor
     * @param hostname          Host name or IP address
     * @param port              Port number
     */
    Host(const String& hostname, uint16_t port);

    /**
     * Constructor
     * In order to work with IPv6 address, enclose address part in square brackets.
     * @param hostAndPort       The host and port definition, in the format ipv4addr:port.
     */
    explicit Host(const String& hostAndPort);

    /**
     * Copy constructor
     * @param other             The other object
     */
    explicit Host(const Host& other);

    /**
     * Move constructor
     * @param other             The other object
     */
    explicit Host(Host&& other) noexcept;

    /**
     * Destructor
     */
    virtual ~Host()
    {}

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
     * Compare to another host
     * @param other             The other object
     * @return true if objects have not equal data
     */
    bool operator!=(const Host& other) const;

    /**
     * Get host name
     * @return host name
     */
    const String& hostname() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_hostname;
    }

    /**
     * Set port number
     * @param p                 Port number
     */
    void port(uint16_t p)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
		setPort(p);
    }

    /**
     * Get port number
     * @return port number
     */
    uint16_t port() const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_port;
    }

    /**
     * Get host name and port as a string.
     * IPv6 addresses are enclosed in square brackets.
     * @param forceAddress      If true then use IP address instead of hostname
     * @return host name and port string
     */
    String toString(bool forceAddress=false) const;

    /**
     * Get host address
     */
    void getAddress(sockaddr_in& address) const
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        memcpy(&address, &m_address, sizeof(address));
    }
};

/**
 * @}
 */

}

#endif
