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

#pragma once

#include <sptk5/Variant.h>

namespace sptk::xdoc {

class Node
    : public Variant
{
public:

    using Nodes = std::vector<Node>;
    using Attributes = std::map<String, String>;
    using iterator = Nodes::iterator;
    using const_iterator = Nodes::const_iterator;

    enum class Type
        : uint8_t
    {
        Null,
        Text,
        Number,
        Boolean,
        Array,
        Object,
        CData,
        Comment,
        ProcessingInstruction
    };

    Node(const String& nodeName = "", Type type = Type::Null);

    String name() const
    {
        return m_name;
    }

    void name(const String& name)
    {
        m_name = name;
    }

    bool is(Type type) const
    {
        return m_type == type;
    }

    Type type() const
    {
        return m_type;
    }

    void type(Type type)
    {
        m_type = type;
    }

    Node& pushNode(const String& name, Type type);

    template<typename T>
    Node& pushValue(const String& name, Type type, const T& value)
    {
        Variant v(value);
        auto& node = pushNode(name, type);
        node.setData(v);
        return node;
    }

    String getAttribute(const String& name) const;

    void setAttribute(const String& name, const String& value);

    Variant& operator[](const String& name)
    {
        return *find(name, true);
    }

    const Variant& operator[](const String& name) const
    {
        auto* pNode = find(name);
        if (pNode == nullptr)
        {
            throw Exception("Element " + name + " doesn't exist");
        }
        return *pNode;
    }

    Variant& operator[](const size_t index)
    {
        return m_nodes[index];
    }

    const Variant& operator[](const size_t index) const
    {
        return m_nodes[index];
    }

    Node* find(const String& name, bool createIfMissing);

    const Node* find(const String& name) const;

private:

    String m_name;
    Type m_type {Type::Null};
    Attributes m_attributes;
    Nodes m_nodes;
};

}
