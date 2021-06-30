/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include "sptk5/wsdl/WSServices.h"

using namespace std;
using namespace sptk;

WSServices::WSServices(const SWSRequest& defaultService)
    : m_services({{"", defaultService}})
{
}

WSServices::WSServices(const WSServices& other)
{
    assign(other);
}

void WSServices::set(const sptk::String& location, const SWSRequest& service)
{
    scoped_lock lock(m_mutex);
    m_services[location] = service;
}

WSRequest& WSServices::get(const sptk::String& location) const
{
    scoped_lock lock(m_mutex);
    auto itor = m_services.find(location);
    if (itor == m_services.end())
    {
        itor = m_services.find("");
    }
    return *itor->second;
}

void WSServices::assign(const WSServices& other)
{
    scoped_lock lock(other.m_mutex);
    m_services = other.m_services;
}
