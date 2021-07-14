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

#include <sptk5/xdoc/Node.h>
#include <sptk5/xdoc/ImportXML.h>
#include <sptk5/xdoc/ExportXML.h>
#include "XPath.h"

using namespace std;
using namespace sptk;
using namespace xdoc;

Node::Node(const String& nodeName, Type type)
    : m_name(nodeName), m_type(type)
{
}

Attributes& Node::attributes()
{
    return m_attributes;
}

const Attributes& Node::attributes() const
{
    return m_attributes;
}

bool Node::hasAttribute(const String& name) const
{
    return m_attributes.has(name);
}

String Node::getAttribute(const String& name, const String& defaultValue) const
{
    return m_attributes.get(name);
}

void Node::setAttribute(const String& name, const String& value)
{
    m_attributes.set(name, value);
}

Node& Node::findOrCreate(const String& name)
{
    if (!is(Type::Object))
    {
        throw Exception("This element is not JSON object");
    }

    for (auto& node: m_nodes)
    {
        if (node.name() == name)
        {
            return node;
        }
    }

    m_nodes.emplace_back();
    auto& newNode = m_nodes.back();
    newNode.name(name);
    newNode.m_parent = this;
    return newNode;
}

Node* Node::find(const String& name, SearchMode searchMode)
{
    if (!is(Type::Object))
    {
        return nullptr;
    }

    // Search for immediate child, first
    for (auto& node: m_nodes)
    {
        if (node.name() == name)
        {
            return &node;
        }
    }

    if (searchMode == SearchMode::Recursive)
    {
        Node* found {nullptr};
        for (auto& node: m_nodes)
        {
            if (node.is(Type::Object) && (found = node.find(name, searchMode)))
            {
                return found;
            }
        }
    }

    return nullptr;
}

const Node* Node::find(const String& name, SearchMode searchMode) const
{
    if (!is(Type::Object))
    {
        throw Exception("This element is not XDocument object");
    }

    // Search for immediate child, first
    for (const auto& node: m_nodes)
    {
        if (node.name() == name)
        {
            return &node;
        }
    }

    if (searchMode == SearchMode::Recursive)
    {
        for (const auto& node: m_nodes)
        {
            const auto* found = node.find(name, searchMode);
            if (found)
            {
                return found;
            }
        }
    }

    return nullptr;
}

Node& Node::pushNode(const String& name, Type type)
{
    if (m_type == Type::Null)
    {
        if (name.empty())
        {
            m_type = Type::Array;
        }
        else
        {
            m_type = Type::Object;
        }
    }
    m_nodes.resize(m_nodes.size() + 1);
    auto& node = m_nodes.back();
    node.name(name);
    node.type(type);
    node.m_parent = this;
    return m_nodes.back();
}

String Node::getString(const String& name) const
{
    auto* node = this;

    if (!name.empty())
    {
        node = find(name);
        if (node == nullptr)
        {
            return String();
        }
    }

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

double Node::getNumber(const String& name) const
{
    if (name.empty())
    {
        return asFloat();
    }

    if (auto* node = find(name);
        node != nullptr)
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

    if (auto* node = find(name);
        node != nullptr)
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

    if (auto* node = find(name);
        node && node->is(Type::Array))
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

    if (auto* node = find(name);
        node && node->is(Type::Object))
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

void Node::load(DataFormat dataFormat, const Buffer& data, bool xmlKeepFormatting)
{
    clear();
    if (dataFormat == DataFormat::JSON)
    {
        importJson(*this, data);
    }
    else
    {
        importXML(data, xmlKeepFormatting);
    }
}

void Node::load(DataFormat dataFormat, const String& data, bool xmlKeepSpaces)
{
    Buffer input(data);

    clear();
    if (dataFormat == DataFormat::JSON)
    {
        importJson(*this, input);
    }
    else
    {
        importXML(input, xmlKeepSpaces);
    }
}

void Node::exportTo(DataFormat dataFormat, Buffer& data, bool formatted) const
{
    if (dataFormat == DataFormat::JSON)
    {
        exportJson(data, formatted);
    }
    else
    {
        ExportXML exporter;
        if (m_parent != nullptr)
        {
            // Exporting single node
            exporter.saveElement(*this, name(), data, formatted ? 2 : 0);
        }
        else
        {
            // Exporting root node of the document
            for (auto& node: m_nodes)
            {
                exporter.saveElement(node, node.name(), data, formatted ? 2 : 0);
            }
        }
    }
}

void Node::importXML(const Buffer& xml, bool xmlKeepSpaces)
{
    ImportXML importer;
    importer.parse(*this, xml.c_str(), xmlKeepSpaces ? ImportXML::Mode::KeepFormatting : ImportXML::Mode::Compact);
}

void Node::exportXML(Buffer& xml, int indent) const
{
    ExportXML exporter;
    for (auto& node: m_nodes)
    {
        exporter.save(node, xml, indent);
    }
}

Node* Node::parent()
{
    return m_parent;
}

void Node::clearChildren()
{
    m_nodes.clear();
}

void Node::select(Node::Vector& selectedNodes, const String& xpath)
{
    selectedNodes.clear();
    NodeSearchAlgorithms::select(selectedNodes, *this, xpath);
}
