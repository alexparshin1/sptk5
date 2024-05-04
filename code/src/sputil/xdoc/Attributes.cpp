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

#include <sptk5/xdoc/Attributes.h>

using namespace sptk;
using namespace xdoc;

String Attributes::get(const String& name, const String& defaultValue) const
{
    for (const auto& [attr, value]: m_items)
    {
        if (attr == name)
        {
            return value;
        }
    }
    return defaultValue;
}

bool Attributes::have(const String& name) const
{
    return std::ranges::any_of(m_items,
                               [&name](const auto& itor) {
                                   return itor.first == name;
                               });
}

Attributes& Attributes::set(const String& name, const String& value)
{
    for (auto& [attr, val]: m_items)
    {
        if (attr == name)
        {
            val = value;
            return *this;
        }
    }
    m_items.emplace_back(name, value);
    return *this;
}
