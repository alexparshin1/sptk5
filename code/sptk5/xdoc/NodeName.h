/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-03-04                                             ║
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

namespace sptk::xdoc {

/**
 * @brief XDoc node name with optional namespace.
 */
class SP_EXPORT NodeName
{
public:
    /**
     * @brief Constructor.
     * @param name              Node name.
     * @param nameSpace         Node namespace.
     */
    NodeName(std::string name, std::string nameSpace)
        : m_name(name)
        , m_nameSpace(nameSpace)
    {
        setQualifiedName();
    }

    /**
     * @brief Constructor.
     * @param name              Node name, short or qualified.
     */
    NodeName(const char* name)
        : NodeName(name == nullptr ? std::string("") : std::string(name))
    {
    }

    /**
     * @brief Constructor.
     * @param name              Node name, short or qualified.
     */
    NodeName(const std::string_view name)
        : m_qualifiedName(name)
    {
        parseQualifiedName();
    }

    /**
     * @brief Constructor.
     * @param name              Node name, short or qualified.
     */
    NodeName(const std::string& name)
        : m_qualifiedName(name)
    {
        parseQualifiedName();
    }

    NodeName(const NodeName& other)
        : m_qualifiedName(other.m_qualifiedName)
    {
        parseQualifiedName();
    }

    NodeName(NodeName&& other)
        : m_qualifiedName(std::move(other.m_qualifiedName))
    {
        parseQualifiedName();
    }

    NodeName& operator=(const NodeName& other)
    {
        if (this != &other)
        {
            m_qualifiedName = other.m_qualifiedName;
            parseQualifiedName();
        }
        return *this;
    }

    NodeName& operator=(NodeName&& other)
    {
        if (this != &other)
        {
            m_qualifiedName = std::move(other.m_qualifiedName);
            parseQualifiedName();
        }
        return *this;
    }

    /**
     * @brief Get node name.
     * @return Node name.
     */
    [[nodiscard]] std::string_view getName() const
    {
        return m_name;
    }

    /**
     * @brief Set the node name.
     * @param name              Node name.
     */
    void setName(std::string name)
    {
        m_name = name;
        setQualifiedName();
    }

    /**
     * @brief Get node namespace.
     * @return Node name space.
     */
    [[nodiscard]] std::string_view getNamespace() const
    {
        return m_nameSpace;
    }

    /**
     * @brief Set node namespace.
     * @param nameSpace         Node name space.
     */
    void setNameSpace(std::string nameSpace)
    {
        m_nameSpace = std::move(nameSpace);
        setQualifiedName();
    }

    /**
     * @brief Get node qualified name.
     * @return Node qualified name.
     */
    [[nodiscard]] const std::string& getQualifiedName() const
    {
        return m_qualifiedName;
    }

    /**
     * @return True if the node name is empty.
     */
    [[nodiscard]] bool empty() const
    {
        return m_name.empty();
    }

    /**
     * @brief Compare the node name to the other node name.
     * @param nodeName          Another node name.
     * @return True if the other node name is matching to this node name.
     */
    [[nodiscard]] bool sameName(const NodeName& nodeName) const
    {
        return operator==(nodeName);
    }

    /**
     * @brief Compare the node name to another node name.
     * @param other             Other node.
     * @return True if another node name is matching to this node name.
     */
    [[nodiscard]] bool operator==(const NodeName& other) const
    {
        return m_qualifiedName == other.m_qualifiedName;
    }

    /**
     * @brief Compare the node name to another node name.
     * @param other             Other node.
     * @return True if another node name is not matching to this node name.
     */
    [[nodiscard]] bool operator!=(const NodeName& other) const
    {
        return m_qualifiedName != other.m_qualifiedName;
    }

private:
    std::string_view m_name {};       ///< Node name.
    std::string_view m_nameSpace {};  ///< Node namespace.
    std::string      m_qualifiedName; ///< Node qualified name.

    /**
     * @brief Parse internal qualified name into namespace and node name.
     */
    void parseQualifiedName()
    {
        if (const auto* pos = strchr(m_qualifiedName.c_str(), ':'))
        {
            const size_t namespaceLength = pos - m_qualifiedName.c_str();
            m_nameSpace = {m_qualifiedName.c_str(), namespaceLength};
            m_name = {m_qualifiedName.c_str() + namespaceLength + 1, m_qualifiedName.length() - namespaceLength - 1};
        }
        else
        {
            m_name = {m_qualifiedName.c_str(), m_qualifiedName.length()};
            m_nameSpace = {};
        }
    }

    /**
     * @brief Set the qualified name after changing the name or namespace.
     */
    void setQualifiedName()
    {
        if (m_nameSpace.empty())
        {
            m_qualifiedName = m_name;
            m_nameSpace = {};
        }
        else
        {
            auto namespaceLength = m_nameSpace.length();
            auto nameLength = m_name.length();
            m_qualifiedName = format("{}:{}", m_nameSpace, m_name);
            m_nameSpace = {m_qualifiedName.c_str(), namespaceLength};
            m_name = {m_qualifiedName.c_str() + namespaceLength + 1, nameLength};
        }
    }
};

} // namespace sptk::xdoc
