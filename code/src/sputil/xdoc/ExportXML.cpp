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

#include <sptk5/Printer.h>
#include <sptk5/xdoc/ExportXML.h>

using namespace std;
using namespace sptk;
using namespace sptk::xdoc;

static const String indentsString(1024, ' ');

inline bool isNodeByName(const String& nodeName)
{
    return !(nodeName[0] == '#' && (nodeName == "#text" || nodeName == "#cdata"));
}

void ExportXML::saveElement(const Node* node, const String& _nodeName, Buffer& buffer, bool formatted, int indent)
{
    const String nodeName = _nodeName.empty() ? "item" : _nodeName;

    const bool isNode = isNodeByName(nodeName);
    const size_t parentSubnodesCount = node->parent() ? node->parent()->nodes().size() : 0;

    if (isNode)
    {
        if (formatted && indent > 0)
        {
            buffer.append(indentsString.c_str(), size_t(indent));
        }
        appendNodeNameAndAttributes(node, nodeName, buffer);
    }
    else
    {
        if (formatted && parentSubnodesCount > 1)
        {
            buffer.append(indentsString.c_str(), size_t(indent));
        }
    }

    const auto& subNodes = node->nodes();
    if (!subNodes.empty())
    {
        if (isNode)
        {
            buffer.append('>');
        }

        if (const bool firstSubNodeIsText = subNodes.front()->name()[0] == '#';
            formatted && (!firstSubNodeIsText || subNodes.size() > 1))
        {
            buffer.append('\n');
        }

        appendSubNodes(node, buffer, formatted, indent);

        if (isNode)
        {
            appendClosingTag(node, buffer, formatted, indent);
        }
    }
    else
    {
        appendNodeEnd(node, nodeName, buffer, isNode);

        if (formatted && isNode)
        {
            buffer.append('\n');
        }
    }
}

void ExportXML::appendNodeNameAndAttributes(const Node* node, const String& nodeName, Buffer& buffer)
{
    switch (node->type())
    {
        case Node::Type::ProcessingInstruction:
            buffer.append("<?", 2);
            break;
        case Node::Type::Comment:
            buffer.append("<!--", 4);
            break;
        default:
            buffer.append('<');
            break;
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
    if (node->type() == Node::Type::Number)
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
        if (node->type() == Node::Type::CData)
        {
            constexpr int cdataTagLength = 9;
            buffer.append("<![CDATA[", cdataTagLength);
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

void ExportXML::appendSubNodes(const Node* node, Buffer& buffer, bool formatted, int indent)
{
    for (const auto& np: node->nodes())
    {
        saveElement(np.get(), np->name(), buffer, formatted, indent + m_indentSpaces);
        if (formatted && node->nodes().size() > 1 && np->name()[0] == '#')
        {
            buffer.append('\n');
        }
    }
}

void ExportXML::appendNodeEnd(const Node* node, const String& nodeName, Buffer& buffer, bool isNode)
{
    if (node->type() == Node::Type::ProcessingInstruction)
    {
        buffer.append("?>", 2);
    }
    else if (node->type() == Node::Type::Comment)
    {
        buffer.append("-->", 3);
    }
    else if (node->type() != Node::Type::Null)
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

void ExportXML::appendClosingTag(const Node* node, Buffer& buffer, bool formatted, int indent)
{
    // output indendation spaces
    if (const bool lastSubNodeIsText = node->nodes().back()->name()[0] == '#';
        formatted && indent > 0 && !lastSubNodeIsText)
    {
        buffer.append(indentsString.c_str(), size_t(indent));
    }

    // output closing tag
    buffer.append("</", 2);
    buffer.append(node->name());
    buffer.append('>');
    if (formatted)
    {
        buffer.append('\n');
    }
}

void ExportXML::saveAttributes(const Node* node, Buffer& buffer)
{
    Buffer real_id;
    Buffer real_val;
    for (const auto& [attr, value]: node->attributes())
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
