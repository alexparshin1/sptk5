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

class Host
{
    std::string m_hostname;
    uint16_t    m_port;

public:

    Host()
        : m_port(0)
    {}

    Host(const std::string& hostname, uint16_t port)
        : m_hostname(hostname), m_port(port)
    {}

    explicit Host(const std::string& hostAndPort);

    explicit Host(const Host& other)
        : m_hostname(other.m_hostname), m_port(other.m_port)
    {}

    explicit Host(Host&& other)
        : m_hostname(move(other.m_hostname)), m_port(other.m_port)
    {
        other.m_port = 0;
    }

    Host& operator = (const Host& other)
    {
        m_hostname = other.m_hostname;
        m_port = other.m_port;
        return *this;
    }

    bool operator == (const Host& other) const
    {
        return m_hostname == other.m_hostname && m_port == other.m_port;
    }

    bool operator != (const Host& other) const
    {
        return m_hostname != other.m_hostname || m_port != other.m_port;
    }

    const std::string& hostname() const {return m_hostname; }

    void port(uint16_t p) { m_port = p; }
    uint16_t port() const {return m_port; }

    std::string toString() const
    {
        std::stringstream str;
        str << m_hostname << ":" << m_port;
        return str.str();
    }

};

}

#endif
