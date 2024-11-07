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
        setQualifiedName();
    }

    NodeName(const char* name)
        : NodeName(String(name))
    {
    }

    NodeName(const String& name)
        : m_qualifiedName(name)
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

    void setName(String name)
    {
        m_name = std::move(name);
        setQualifiedName();
    }

    [[nodiscard]] const String& getNameSpace() const
    {
        return m_nameSpace;
    }

    void setNameSpace(String nameSpace)
    {
        m_nameSpace = std::move(nameSpace);
    }

    [[nodiscard]] const String& getQualifiedName() const
    {
        return m_qualifiedName;
    }

    bool empty() const
    {
        return m_name.empty();
    }

    bool sameName(const NodeName& nodeName)
    {
        return operator==(nodeName);
    }

    [[nodiscard]] bool operator==(const NodeName& other) const
    {
        return m_qualifiedName == other.m_qualifiedName;
    }

    [[nodiscard]] bool operator!=(const NodeName& other) const
    {
        return m_qualifiedName != other.m_qualifiedName;
    }

private:
    String m_name;          ///< Node name
    String m_nameSpace;     ///< Node namespace
    String m_qualifiedName; ///< Node qualified name

    /**
     * @brief Set qualified name after changing name or name space
     */
    void setQualifiedName()
    {
        if (m_nameSpace.empty())
        {
            m_qualifiedName = m_name;
        }
        else
        {
            m_qualifiedName = m_nameSpace + ":" + m_name;
        }
    }
};

} // namespace sptk::xdoc
