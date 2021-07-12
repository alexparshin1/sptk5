#include "ExportXML.h"

using namespace std;
using namespace sptk;
using namespace sptk::xdoc;

static const String emptyString;
static const String indentsString(1024, ' ');

static constexpr int cdataStartMarkerLength = 9;
static constexpr int cdataEndMarkerLength = 3;

void ExportXML::saveElement(const Node& node, const String& nodeName, Buffer& buffer, int indent)
{
    bool isNode = !(nodeName[0] == '#' && (nodeName == "#text" || nodeName == "#cdata"));

    if (isNode)
    {
        buffer.append('<');

        if (node.type() == Node::Type::ProcessingInstruction)
        {
            buffer.append('?');
        }

        buffer.append(nodeName);
        if (!node.attributes().empty())
        {
            // Output attributes
            saveAttributes(node, buffer);
        }
    }

    if (!node.empty())
    {
        if (isNode)
        {
            buffer.append('>');
        }

        bool only_cdata = true;
        if (const auto& nd = node.begin();
            node.size() == 1 && nd->type() == Node::Type::Text)
        {
        }
        else
        {
            only_cdata = false;
            if (indent)
            {
                buffer.append('\n');
            }
        }
        appendSubNodes(node, buffer, indent, only_cdata);
        if (isNode)
        {
            appendClosingTag(node, buffer, indent, only_cdata);
        }
    }
    else
    {
        //LEAF
        if (node.type() == Node::Type::ProcessingInstruction)
        {
            buffer.append("?>", 2);
        }
        else if (!node.isNull())
        {
            if (isNode)
            {
                buffer.append('>');
            }
            if (node.is(Node::Type::Number))
            {
                double dvalue = node.asFloat();
                long lvalue = long(dvalue);
                if (dvalue == double(lvalue))
                {
                    buffer.append(to_string(lvalue));
                }
                else
                {
                    buffer.append(node.asString());
                }
            }
            else
            {
                if (node.is(Node::Type::CData))
                {
                    buffer.append("<![CDATA[", 9);
                    buffer.append(node.asString());
                    buffer.append("]]>", 3);
                }
                else
                {
                    //buffer.append(node.asString());
                    m_docType.encodeEntities(node.asString().c_str(), buffer);
                }
            }
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

        if (indent)
        {
            buffer.append('\n');
        }
    }
}

void ExportXML::appendSubNodes(const Node& node, Buffer& buffer, int indent, bool only_cdata)
{
    for (const auto& np: node)
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
            saveElement(np, np.name(), buffer, newIndent);
            if (indent && buffer.data()[buffer.bytes() - 1] != '\n')
            {
                buffer.append('\n');
            }
        }
    }
}

void ExportXML::appendClosingTag(const Node& node, Buffer& buffer, int indent, bool only_cdata) const
{
    // output indendation spaces
    if (!only_cdata && indent > 0)
    {
        buffer.append(indentsString.c_str(), size_t(indent));
    }

    // output closing tag
    buffer.append("</", 2);
    buffer.append(node.name());
    buffer.append('>');
    if (indent)
    {
        buffer.append('\n');
    }
}

void ExportXML::saveAttributes(const Node& node, Buffer& buffer)
{
    Buffer real_id;
    Buffer real_val;
    for (const auto&[attr, value]: node.attributes())
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

void ExportXML::save(const Node& node, Buffer& buffer, int indent)
{
    // output indendation spaces
    if (indent > 0)
    {
        buffer.append(indentsString.c_str(), size_t(indent));
    }

    const auto& nodeName = node.name();
    String value(node.asString());

    // depending on the nodetype, do output
    switch (node.type())
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
            saveElement(node, nodeName, buffer, indent);
            break;
    }
}
