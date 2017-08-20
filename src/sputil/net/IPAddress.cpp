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

#include <sptk5/net/IPAddress.h>

using namespace std;
using namespace sptk;

IPAddress::IPAddress()
{
    memset(&m_address, 0, sizeof(m_address));
}

IPAddress::IPAddress(const sockaddr& address)
{
    char buffer[64];
    memcpy(&m_address, &address, addressLength(address));
    m_addressStr = inet_ntop(m_address.generic.sa_family, &m_address.generic, buffer, sizeof(buffer));
}

IPAddress::IPAddress(const IPAddress& other)
{
    memcpy(&m_address, &other.m_address, sizeof(m_address));
    m_addressStr = other.m_addressStr;
}

IPAddress& IPAddress::operator=(const IPAddress& other)
{
    memcpy(&m_address, &other.m_address, sizeof(m_address));
    m_addressStr = other.m_addressStr;
    return *this;
}
