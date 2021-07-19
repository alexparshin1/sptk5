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

#include <sptk5/xdoc/ExportXML.h>

using namespace std;
using namespace sptk;
using namespace sptk::xdoc;

static const String emptyString;
static const String indentsString(1024, ' ');

static constexpr int cdataStartMarkerLength = 9;
static constexpr int cdataEndMarkerLength = 3;

inline bool isNodeByName(const String& nodeName)
{
    return !(nodeName[0] == '#' && (nodeName == "#text" || nodeName == "#cdata"));
}

void ExportXML::saveElement(const Node* node, const String& _nodeName, Buffer& buffer, int indent)
{
    String nodeName = _nodeName.empty() ? "item" : _nodeName;

    bool isNode = isNodeByName(nodeName);

    if (isNode)
    {
        appendNodeNameAndAttributes(node, nodeName, buffer);
    }

    if (!node->empty())
    {
        if (isNode)
        {
            buffer.append('>');
        }

        bool only_cdata = false;
        if (const auto& nd = node->begin();
            node->size() == 1 && ((*nd)->type() == Node::Type::Text || (*nd)->type() == Node::Type::CData))
        {
            only_cdata = node->name() == "#text" || node->name() == "#cdata";
        }

        if (indent && !only_cdata)
        {
            buffer.append('\n');
        }

        appendSubNodes(node, buffer, indent, only_cdata);

        if (isNode)
        {
            appendClosingTag(node, buffer, indent, only_cdata);
        }
    }
    else
    {
        appendNodeEnd(node, nodeName, buffer, isNode);

        if (indent)
        {
            buffer.append('\n');
        }
    }
}

void ExportXML::appendNodeNameAndAttributes(const Node* node, const String& nodeName, Buffer& buffer)
{
    buffer.append('<');

    if (node->type() == Node::Type::ProcessingInstruction)
    {
        buffer.append('?');
    }

    buffer.append(nodeName);
    if (!node->attributes().empty())
    {
        // Output attributes
        saveAttributes(node, buffer);
    }
}

Buffer& ExportXML::appendNodeContent(const Node* node, Buffer& buffer)
{
    if (node->is(Node::Type::Number))
    {
        auto dvalue = node->getValue().asFloat();
        auto lvalue = long(dvalue);
        if (dvalue == double(lvalue))
        {
            buffer.append(to_string(lvalue));
        }
        else
        {
            buffer.append(node->getValue().asString());
        }
    }
    else
    {
        if (node->is(Node::Type::CData))
        {
            buffer.append("<![CDATA[", 9);
            buffer.append(node->getValue().asString());
            buffer.append("]]>", 3);
        }
        else
        {
            m_docType.encodeEntities(node->getValue().asString().c_str(), buffer);
        }
    }
    return buffer;
}

void ExportXML::appendSubNodes(const Node* node, Buffer& buffer, int indent, bool only_cdata)
{
    for (const auto& np: *node)
    {
        if (only_cdata)
        {
            save(np, buffer, -1);
        }
        else
        {
            int newIndent = 0;
            if (indent)
            {
                newIndent = indent + m_indentSpaces;
            }
            saveElement(np.get(), np->name(), buffer, newIndent);
            if (indent && buffer.data()[buffer.bytes() - 1] != '\n')
            {
                buffer.append('\n');
            }
        }
    }
}

void ExportXML::appendNodeEnd(const Node* node, const String& nodeName, Buffer& buffer, bool isNode)
{
    if (node->type() == Node::Type::ProcessingInstruction)
    {
        buffer.append("?>", 2);
    }
    else if (!node->is(Node::Type::Null))
    {
        if (isNode)
        {
            buffer.append('>');
        }
        buffer = appendNodeContent(node, buffer);
        if (isNode)
        {
            buffer.append("</", 2);
            buffer.append(nodeName);
            buffer.append('>');
        }
    }
    else
    {
        buffer.append("/>", 2);
    }
}

void ExportXML::appendClosingTag(const Node* node, Buffer& buffer, int indent, bool only_cdata) const
{
    // output indendation spaces
    if (!only_cdata && indent > 0)
    {
        buffer.append(indentsString.c_str(), size_t(indent));
    }

    // output closing tag
    buffer.append("</", 2);
    buffer.append(node->name());
    buffer.append('>');
    if (indent)
    {
        buffer.append('\n');
    }
}

void ExportXML::saveAttributes(const Node* node, Buffer& buffer)
{
    Buffer real_id;
    Buffer real_val;
    for (const auto&[attr, value]: node->attributes())
    {
        real_id.bytes(0);
        real_val.bytes(0);
        if (!m_docType.encodeEntities(attr.c_str(), real_id))
        {
            real_id = attr;
        }
        if (!m_docType.encodeEntities(value.c_str(), real_val))
        {
            real_val = value;
        }

        buffer.append(' ');
        buffer.append(real_id);
        buffer.append("=\"");
        buffer.append(real_val);
        buffer.append("\"");
    }
}

void ExportXML::save(const SNode& node, Buffer& buffer, int indent)
{
    // output indendation spaces
    if (indent > 0)
    {
        buffer.append(indentsString.c_str(), size_t(indent));
    }

    const auto& nodeName = node->name();
    String value(node->getValue().asString());

    // depending on the nodetype, do output
    switch (node->type())
    {
        case Node::Type::Text:
            if (value.substr(0, cdataStartMarkerLength) == "<![CDATA[" &&
                value.substr(value.length() - cdataEndMarkerLength) == "]]>")
            {
                buffer.append(value);
            }
            else
            {
                m_docType.encodeEntities(value.c_str(), buffer);
            }
            break;

        case Node::Type::CData:
            // output all subnodes
            buffer.append("<![CDATA[" + value + "]]>");
            if (indent)
            {
                buffer.append("\n", 1);
            }
            break;

        case Node::Type::Comment:
            // output all subnodes
            buffer.append("<!-- " + value + " -->");
            if (indent)
            {
                buffer.append("\n", 1);
            }
            break;

        default:
            saveElement(node.get(), nodeName, buffer, indent);
            break;
    }
}
