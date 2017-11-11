/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       cnet - description                                     ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday September 13 2017                            ║
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

#ifndef __HOST_H__
#define __HOST_H__

#include <sstream>

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
    std::string m_hostname;     ///< Host name or IP address
    uint16_t    m_port;         ///< Port number

public:

    /**
     * Default constructor
     */
    Host() : m_port(0)
    {}

    /**
     * Constructor
     * @param hostname Host name or IP address
     * @param port Port number
     */
    Host(const std::string& hostname, uint16_t port)
    : m_hostname(hostname), m_port(port)
    {}

    /**
     * Constructor
     * In order to work with IPv6 address, enclose address part in square brackets.
     * @param hostAndPort The host and port definition, in the format <ipv4addr>:<port>.
     */
    explicit Host(const std::string& hostAndPort);

    /**
     * Copy constructor
     * @param other The other object
     */
    explicit Host(const Host& other)
    : m_hostname(other.m_hostname), m_port(other.m_port)
    {}

    /**
     * Move constructor
     * @param other The other object
     */
    explicit Host(Host&& other)
    : m_hostname(move(other.m_hostname)), m_port(other.m_port)
    {
        other.m_port = 0;
    }

    /**
     * Destructor
     */
    virtual ~Host() {}

    /**
     * Assign from another host
     * @param other The other object
     */
    Host& operator = (const Host& other)
    {
        m_hostname = other.m_hostname;
        m_port = other.m_port;
        return *this;
    }

    /**
     * Move assignment from another host
     * @param other The other object
     */
    Host& operator = (Host&& other)
    {
        m_hostname = std::move(other.m_hostname);
        m_port = other.m_port;
        return *this;
    }

    /**
     * Compare to another host
     * @param other The other object
     */
    bool operator == (const Host& other) const
    {
        return m_hostname == other.m_hostname && m_port == other.m_port;
    }

    /**
     * Compare to another host
     * @param other The other object
     */
    bool operator != (const Host& other) const
    {
        return m_hostname != other.m_hostname || m_port != other.m_port;
    }

    /**
     * Get host name
     * @return host name
     */
    const std::string& hostname() const { return m_hostname; }

    /**
     * Set port number
     * @param p Port number
     */
    void port(uint16_t p) { m_port = p; }

    /**
     * Get port number
     * @return port number
     */
    uint16_t port() const { return m_port; }

    /**
     * Get host name and port as a string.
     * IPv6 addresses are enclosed in square brackets.
     * @return host name and port string
     */
    std::string toString() const
    {
        std::stringstream str;
        if (hostname().find(':') != std::string::npos)
            str << "[" << m_hostname << "]:" << m_port;
        else
            str << m_hostname << ":" << m_port;
        return str.str();
    }

};

/**
 * @}
 */

}

#endif
