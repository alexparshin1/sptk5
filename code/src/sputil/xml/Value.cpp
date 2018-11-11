/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Value.cpp - description                                ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <sptk5/cxml>

using namespace sptk;
using namespace sptk::xml;

Value::operator bool() const
{
    if (m_value.empty())
        return false;
    char ch = m_value.c_str()[0];
    const char *p = strchr("TtYy1", ch);
    return p != nullptr;
}

Value& Value::operator =(bool v)
{
    if (v)
        m_value.assign("Y", 1);
    else
        m_value.assign("N", 1);
    return *this;
}

Value& Value::operator =(int32_t v)
{
    char buff[64];
    auto sz = (uint32_t) snprintf(buff, sizeof(buff), "%i", v);
    m_value.assign(buff, sz);
    return *this;
}

Value& Value::operator =(uint32_t v)
{
    char buff[64];
    auto sz = (uint32_t) snprintf(buff, sizeof(buff), "%u", v);
    m_value.assign(buff, sz);
    return *this;
}

Value& Value::operator =(int64_t v)
{
    char buff[64];
#ifndef _WIN32
    auto sz = (uint32_t) snprintf(buff, sizeof(buff), "%li", v);
#else
    uint32_t sz = (uint32_t) snprintf(buff, sizeof(buff), "%lli", v);
#endif
    m_value.assign(buff, sz);
    return *this;
}

Value& Value::operator =(uint64_t v)
{
    char buff[64];
#ifndef _WIN32
    auto sz = (uint32_t) snprintf(buff, sizeof(buff), "%lu", v);
#else
    uint32_t sz = (uint32_t) snprintf(buff, sizeof(buff), "%llu", v);
#endif
    m_value.assign(buff, sz);
    return *this;
}

Value& Value::operator =(double v)
{
    char buff[64];
    auto sz = (uint32_t) snprintf(buff, sizeof(buff), "%f", v);
    m_value.assign(buff, sz);
    return *this;
}
