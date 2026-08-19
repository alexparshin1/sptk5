/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/xdoc/ExportXML.h>

using namespace std;
using namespace sptk;
using namespace sptk::xdoc;

static const String indentsString(1024, ' ');

inline bool isNodeByName(const String& nodeName)
{
    return !(nodeName[0] == '#' && (nodeName == "#text" || nodeName == "#cdata"));
}

void ExportXML::saveElement(const Node* node, const String& _nodeName, Buffer& buffer, const bool formatted, const int indent)
{
    const String nodeName = _nodeName.empty() ? "item" : _nodeName;

    const auto isNode = isNodeByName(nodeName);
    const auto parentSubnodesCount = node->parent() ? node->parent()->nodes().size() : 0;

    if (isNode)
    {
        if (formatted && indent > 0)
        {
            buffer.append(indentsString.c_str(), static_cast<size_t>(indent));
        }
        appendNodeNameAndAttributes(node, nodeName, buffer);
    }
    else
    {
        if (formatted && parentSubnodesCount > 1)
        {
            buffer.append(indentsString.c_str(), static_cast<size_t>(indent));
        }
    }

    if (const auto& subNodes = node->nodes();
        !subNodes.empty())
    {
        if (isNode)
        {
            buffer.append('>');
        }

        const auto firstSubnodeName = subNodes.front()->getName();
        if (const auto firstSubNodeIsText = firstSubnodeName.empty() ? false : subNodes.front()->getName()[0] == '#';
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
        const auto dvalue = node->getValue().asFloat();
        if (const auto lvalue = static_cast<long>(dvalue);
            dvalue == static_cast<double>(lvalue))
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
            constexpr auto cdataTagLength = 9;
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

void ExportXML::appendSubNodes(const Node* node, Buffer& buffer, const bool formatted, const int indent)
{
    for (const auto& np: node->nodes())
    {
        saveElement(np.get(), np->getQualifiedName(), buffer, formatted, indent + m_indentSpaces);
        if (formatted && node->nodes().size() > 1 && np->getName()[0] == '#')
        {
            buffer.append('\n');
        }
    }
}

void ExportXML::appendNodeEnd(const Node* node, const String& nodeName, Buffer& buffer, const bool isNode)
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

void ExportXML::appendClosingTag(const Node* node, Buffer& buffer, const bool formatted, const int indent)
{
    // output indendation spaces
    const auto lastSubnodeName = node->nodes().back()->getName();
    if (const auto lastSubNodeIsText = lastSubnodeName.empty() ? false : node->nodes().back()->getName()[0] == '#';
        formatted && indent > 0 && !lastSubNodeIsText)
    {
        buffer.append(indentsString.c_str(), static_cast<size_t>(indent));
    }

    // output closing tag
    buffer.append("</", 2);
    buffer.append(node->getQualifiedName());
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
