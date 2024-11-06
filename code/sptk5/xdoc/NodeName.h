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
#include <sptk5/String.h>

namespace sptk::xdoc {

class SP_EXPORT NodeName
{
public:
    NodeName(String name, String nameSpace)
        : m_name(std::move(name))
        , m_nameSpace(std::move(nameSpace))
    {
    }

    NodeName(const String& name)
    {
        const auto* pos = strchr(name.c_str(), ':');
        if (pos)
        {
            m_nameSpace = std::string(name.c_str(), pos - name.c_str());
            m_name = name.substr(pos - name.c_str() + 1);
        }
        else
        {
            m_name = name;
        }
    }

    NodeName(const NodeName& other) = default;
    NodeName(NodeName&& other) = default;
    NodeName& operator=(const NodeName& other) = default;
    NodeName& operator=(NodeName&& other) = default;

    [[nodiscard]] const String& getName() const
    {
        return m_name;
    }

    [[nodiscard]] const String& getNameSpace() const
    {
        return m_nameSpace;
    }

    void setNameSpace(String nameSpace)
    {
        m_nameSpace = std::move(nameSpace);
    }

    [[nodiscard]] String toString() const
    {
        if (m_nameSpace.empty())
        {
            return m_name;
        }
        return m_nameSpace + ":" + m_name;
    }

    [[nodiscard]] bool operator==(const NodeName& other) const = default;
    [[nodiscard]] bool operator!=(const NodeName& other) const = default;

    bool operator==(const String& other) const
    {
        return toString() == other;
    }

    bool operator!=(const String& other) const
    {
        return toString() != other;
    }

private:
    String m_name;
    String m_nameSpace;
};

} // namespace sptk::xdoc
