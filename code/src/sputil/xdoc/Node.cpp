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

#include "Node.h"

using namespace std;
using namespace sptk;
using namespace xdoc;

Node::Node(const String& nodeName)
    : m_name(nodeName)
{

}

String Node::getAttribute(const String& name) const
{
    const auto itor = m_attributes.find(name);
    if (itor == m_attributes.end())
    {
        return String();
    }
    return itor->second;
}

void Node::setAttribute(const String& name, const String& value)
{
    m_attributes[name] = value;
}

Node* Node::find(const String& name, bool createIfMissing)
{
    for (auto& node: m_nodes)
    {
        if (node.name() == name)
        {
            return &node;
        }
    }

    if (createIfMissing)
    {
        m_nodes.emplace_back();
        return &m_nodes.back();
    }

    return nullptr;
}

const Node* Node::find(const String& name) const
{
    for (const auto& node: m_nodes)
    {
        if (node.name() == name)
        {
            return &node;
        }
    }

    return nullptr;
}
