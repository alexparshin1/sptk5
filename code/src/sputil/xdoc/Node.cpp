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

Node::Node(const String& nodeName, Type type)
    : m_name(nodeName), m_type(type)
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
    if (!is(Type::Object))
    {
        throw Exception("This element is not JSON object");
    }

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
        m_nodes.back().name(name);
        return &m_nodes.back();
    }
    else
    {
        throw Exception("");
    }

    return nullptr;
}

const Node* Node::find(const String& name) const
{
    if (!is(Type::Object))
    {
        throw Exception("This element is not JSON object");
    }

    for (const auto& node: m_nodes)
    {
        if (node.name() == name)
        {
            return &node;
        }
    }

    return nullptr;
}

Node& Node::pushNode(const String& name, Type type)
{
    m_nodes.resize(m_nodes.size() + 1);
    auto& node = m_nodes.back();
    node.name(name);
    node.type(type);
    return m_nodes.back();
}

String Node::getString(const String& name) const
{
    if (name.empty())
    {
        return asString();
    }

    auto* node = find(name);
    if (node)
    {
        if (node->is(Type::Number))
        {
            auto dvalue = node->asFloat();
            auto ivalue = node->asInt64();
            if (dvalue == double(ivalue))
            {
                return int2string(ivalue);
            }
            else
            {
                return double2string(dvalue);
            }
        }
        return node->asString();
    }

    return String();
}

double Node::getNumber(const String& name) const
{
    if (name.empty())
    {
        return asFloat();
    }

    auto* node = find(name);
    if (node)
    {
        return node->asFloat();
    }

    return 0;
}

bool Node::getBoolean(const String& name) const
{
    if (name.empty())
    {
        return asBool();
    }

    auto* node = find(name);
    if (node)
    {
        return node->asBool();
    }

    return false;
}

const Node::Nodes& Node::getArray(const String& name) const
{
    static const Nodes emptyNodes;

    if (name.empty())
    {
        return m_nodes;
    }

    auto* node = find(name);
    if (node && node->is(Type::Array))
    {
        return node->m_nodes;
    }

    return emptyNodes;
}

const Node& Node::getObject(const String& name) const
{
    static const Node emptyNode;

    if (name.empty())
    {
        return *this;
    }

    auto* node = find(name);
    if (node && node->is(Type::Object))
    {
        return *node;
    }

    return emptyNode;
}

void Node::clear()
{
    type(Type::Object);
    m_nodes.clear();
    m_attributes.clear();
}

Node& Node::add_object(const String& name)
{
    return pushNode(name, Type::Object);
}

Node& Node::add_array(const String& name)
{
    return pushNode(name, Type::Array);
}

Node& Node::push_object()
{
    return pushNode("", Type::Object);
}

bool Node::remove(const String& name)
{
    for (auto node = m_nodes.begin(); node != m_nodes.end(); ++node)
    {
        if (node->name() == name)
        {
            m_nodes.erase(node);
            return true;
        }
    }
    return false;
}

size_t Node::size() const
{
    return m_nodes.size();
}

void Node::load(Node::DataFormat dataFormat, const Buffer& data)
{
    if (dataFormat == DataFormat::JSON)
    {
        clear();
        importJson(*this, data);
    }
}

void Node::exportTo(Node::DataFormat dataFormat, Buffer& data, bool formatted) const
{
    if (dataFormat == DataFormat::JSON)
    {
        exportJson(data, formatted);
    }
}
