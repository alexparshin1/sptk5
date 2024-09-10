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

#include "XPath.h"
#include <sptk5/xdoc/ExportJSON.h>
#include <sptk5/xdoc/ExportXML.h>
#include <sptk5/xdoc/ImportXML.h>

#include <utility>

using namespace std;
using namespace sptk;
using namespace xdoc;

Node::Type Node::variantTypeToNodeType(VariantDataType type)
{
    using enum Type;
    switch (type)
    {
        using enum VariantDataType;
        case VAR_NONE:
            return Null;

        case VAR_INT:
        case VAR_FLOAT:
        case VAR_IMAGE_NDX:
        case VAR_INT64:
            return Number;

        case VAR_MONEY:
        case VAR_STRING:
        case VAR_TEXT:
        case VAR_BUFFER:
        case VAR_DATE:
        case VAR_DATE_TIME:
        case VAR_IMAGE_PTR:
            return Text;

        case VAR_BOOL:
            return Boolean;

        default:
            break;
    }

    return Null;
}

Node::Node(String nodeName, Type type)
    : m_name(std::move(nodeName))
    , m_type(type)
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

SNode Node::findOrCreate(const String& name)
{
    if (name.empty())
    {
        throw Exception("Name can't be empty");
    }

    if (m_type == Type::Null)
    {
        m_type = Type::Object;
    }

    if (type() != Type::Object)
    {
        throw Exception("This element is not an Object");
    }

    for (auto& node: m_nodes)
    {
        if (node->name() == name)
        {
            return node;
        }
    }

    const auto newNode = make_shared<Node>(name);
    newNode->m_parent = shared_from_this();
    m_nodes.push_back(newNode);
    return m_nodes.back();
}

SNode Node::findFirst(const String& name, SearchMode searchMode) const
{
    if (type() != Type::Object && type() != Type::Array)
    {
        return nullptr;
    }

    // Search for immediate child, first
    for (const auto& node: m_nodes)
    {
        if (node->name() == name)
        {
            return node;
        }
    }

    if (searchMode == SearchMode::Recursive)
    {
        for (const auto& node: m_nodes)
        {
            if (auto found = node->findFirst(name, searchMode))
            {
                return found;
            }
        }
    }

    return nullptr;
}

SNode Node::pushNode(const String& name, Type type)
{
    using enum Type;
    if (m_type == Null)
    {
        if (name.empty())
        {
            m_type = Array;
        }
        else
        {
            m_type = Object;
        }
    }
    const auto node = make_shared<Node>(name, type);
    m_nodes.push_back(node);
    node->m_parent = shared_from_this();
    return m_nodes.back();
}

String Node::getString(const String& name) const
{
    const auto& node = name.empty() ? shared_from_this() : findFirst(name);

    if (node == nullptr)
    {
        return {};
    }

    if (node->type() == Type::Number)
    {
        const auto doubleValue = node->m_value.asFloat();

        if (const auto intValue = node->m_value.asInt64();
            doubleValue == static_cast<double>(intValue))
        {
            return int2string(intValue);
        }

        return double2string(doubleValue);
    }

    return node->m_value.asString();
}

namespace {
void getTextRecursively(const Node* node, Buffer& output)
{
    if (node->type() != Node::Type::Comment)
    {
        output.append(node->getString());
        for (const auto& child: node->nodes())
        {
            getTextRecursively(child.get(), output);
        }
    }
}
} // namespace

String Node::getText(const String& name) const
{
    const Node* node = this;
    if (!name.empty())
    {
        const auto found = findFirst(name);
        if (!found)
        {
            return {};
        }
        node = found.get();
    }

    Buffer textInSubNodes;
    getTextRecursively(node, textInSubNodes);

    return textInSubNodes.c_str();
}

double Node::getNumber(const String& name) const
{
    if (name.empty())
    {
        return m_value.asFloat();
    }

    if (const auto& node = findFirst(name);
        node != nullptr)
    {
        return node->m_value.asFloat();
    }

    return 0;
}

bool Node::getBoolean(const String& name) const
{
    if (name.empty())
    {
        return m_value.asBool();
    }

    if (const auto& node = findFirst(name);
        node != nullptr)
    {
        return node->m_value.asBool();
    }

    return false;
}

const Node::Nodes& Node::nodes(const String& name) const
{
    static constexpr Nodes emptyNodes;

    if (name.empty())
    {
        return m_nodes;
    }

    if (const auto& node = findFirst(name);
        node && node->type() == Type::Array)
    {
        return node->m_nodes;
    }

    return emptyNodes;
}

void Node::clear()
{
    type(Type::Object);
    for (auto& node: m_nodes)
    {
        node->clear();
    }
    m_nodes.clear();
    m_attributes.clear();
}

SNode xdoc::Node::pushValue(const String& name, const Variant& value, Node::Type type)
{
    Node::Type actualType(type);
    if (type == Node::Type::Null && !value.isNull())
    {
        actualType = variantTypeToNodeType(value.dataType());
    }
    auto node = pushNode(name, actualType);
    node->m_value = value;
    return node;
}

SNode xdoc::Node::pushValue(const Variant& value, Node::Type type)
{
    Node::Type actualType(type);
    if (type == Node::Type::Null && !value.isNull())
    {
        actualType = variantTypeToNodeType(value.dataType());
    }
    auto node = pushNode("", actualType);
    node->m_value = value;
    return node;
}

bool Node::remove(const String& name)
{
    bool found = false;
    for (auto node = m_nodes.begin(); node != m_nodes.end();)
    {
        if ((*node)->name() == name)
        {
            node = m_nodes.erase(node);
            found = true;
        }
        else
        {
            ++node;
        }
    }
    return found;
}

bool Node::remove(const SNode& _node)
{
    return erase_if(m_nodes, [&](const auto& node)
                    {
                        return node.get() == _node.get();
                    });
}

namespace {
void importXML(const SNode& node, const Buffer& xml, bool xmlKeepSpaces)
{
    ImportXML importer;
    importer.parse(node, xml.c_str(), xmlKeepSpaces ? ImportXML::Mode::KeepFormatting : ImportXML::Mode::Compact);
}
} // namespace

void Node::load(DataFormat dataFormat, const Buffer& data, bool xmlKeepFormatting)
{
    clear();
    if (dataFormat == DataFormat::JSON)
    {
        const auto node = shared_from_this();
        importJson(node, data);
    }
    else
    {
        const auto node = shared_from_this();
        importXML(node, data, xmlKeepFormatting);
    }
}

void Node::load(DataFormat dataFormat, const String& data, bool xmlKeepFormatting)
{
    const Buffer input(data);

    clear();
    if (dataFormat == DataFormat::JSON)
    {
        const auto node = shared_from_this();
        importJson(node, input);
    }
    else
    {
        const auto node = shared_from_this();
        importXML(node, input, xmlKeepFormatting);
    }
}

void Node::exportTo(DataFormat dataFormat, Buffer& data, bool formatted) const
{
    if (dataFormat == DataFormat::JSON)
    {
        ExportJSON::exportToJSON(this, data, formatted);
    }
    else
    {
        ExportXML exporter;
        if (m_parent != nullptr)
        {
            // Exporting single node
            exporter.saveElement(this, name(), data, formatted, 0);
        }
        else
        {
            // Exporting root node of the document
            for (const auto& node: m_nodes)
            {
                exporter.saveElement(node.get(), node->name(), data, formatted, 0);
            }
        }
    }
}

void Node::exportTo(DataFormat dataFormat, ostream& stream, bool formatted) const
{
    Buffer output;
    exportTo(dataFormat, output, formatted);
    stream << output.c_str();
}

void Node::clearChildren()
{
    m_nodes.clear();
}

Node::Vector Node::select(const String& xpath)
{
    Node::Vector selectedNodes;

    selectedNodes.clear();
    const auto node = shared_from_this();
    NodeSearchAlgorithms::select(selectedNodes, node, xpath);

    return selectedNodes;
}

void Node::clone(const SNode& destination, const SNode& source)
{
    Buffer content;
    source->exportTo(DataFormat::JSON, content, false);
    destination->load(DataFormat::JSON, content, false);
}

bool xdoc::isBoolean(const String& str)
{
    static const RegularExpression isInteger(R"(^(true|false)$)");

    return isInteger.matches(str);
}

bool xdoc::isInteger(const String& str)
{
    static const RegularExpression isInteger(R"(^(0|[\+\-]?[1-9]\d*)$)");

    return isInteger.matches(str);
}

bool xdoc::isFloat(const String& str)
{
    static const RegularExpression isNumber(R"(^[\+\-]?(0?\.|[1-9]\d*\.)\d+(e[\+\-]?\d+)?$)", "i");

    return isNumber.matches(str);
}
