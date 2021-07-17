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
#include <sptk5/RegularExpression.h>

using namespace std;
using namespace sptk;
using namespace sptk::xdoc;

static String jsonEscape(const String& text)
{
    String result;

    size_t position = 0;

    for (;;)
    {
        size_t pos = text.find_first_of("\"\\\b\f\n\r\t", position);
        if (pos == string::npos)
        {
            if (position == 0)
            {
                return text;
            }
            result += text.substr(position);
            break;
        }
        result += text.substr(position, pos - position);
        switch (text[pos])
        {
            case '"':
                result += "\\\"";
                break;
            case '\\':
                result += "\\\\";
                break;
            case '/':
                result += "\\/";
                break;
            case '\b':
                result += "\\b";
                break;
            case '\f':
                result += "\\f";
                break;
            case '\n':
                result += "\\n";
                break;
            case '\r':
                result += "\\r";
                break;
            case '\t':
                result += "\\t";
                break;
            default:
                throw Exception("Unknown escape character");
        }
        position = pos + 1;
    }

    return result;
}

void Node::exportJsonValueTo(ostream& stream, bool formatted, size_t indent) const
{
    String indentSpaces;
    String newLineChar;
    String firstElement;
    String betweenElements(",");

    if (formatted && (is(Type::Array) || is(Type::Object)))
    {
        if (indent)
        {
            indentSpaces = string(indent, ' ');
        }
        newLineChar = "\n";
        firstElement = "\n  " + indentSpaces;
        betweenElements = ",\n  " + indentSpaces;
    }

    auto saveFlags = stream.flags();

    double dNumber;
    int64_t iNumber;
    switch (type())
    {
        case Type::Number:
            iNumber = asInt64();
            dNumber = asFloat();
            if (double(iNumber) == dNumber)
            {
                stream << fixed << iNumber;
            }
            else
            {
                stream << asString();
            }
            break;

        case Type::Text:
            stream << "\"" << jsonEscape(asString()) << "\"";
            break;

        case Type::CData:
            stream << "\"" << jsonEscape(asString()) << "\"";
            break;

        case Type::Boolean:
            stream << (asBool() ? "true" : "false");
            break;

        case Type::Array:
            exportJsonArray(stream, formatted, indent, firstElement, betweenElements, newLineChar, indentSpaces);
            break;

        case Type::Object:
            exportJsonObject(stream, formatted, indent, firstElement, betweenElements, newLineChar, indentSpaces);
            break;

        default:
            stream << "null";
            break;
    }
    stream.flags(saveFlags);
}

void Node::exportJsonArray(ostream& stream, bool formatted, size_t indent, const String& firstElement,
                           const String& betweenElements, const String& newLineChar, const String& indentSpaces) const
{
    stream << "[";
    if (is(Type::Array))
    {
        bool first = true;
        const auto& array = m_nodes;
        if (array.empty())
        {
            stream << "]";
            return;
        }
        for (const auto& element: array)
        {
            if (first)
            {
                first = false;
                stream << firstElement;
            }
            else
            {
                stream << betweenElements;
            }
            element->exportJsonValueTo(stream, formatted, indent + 2);
        }
    }
    stream << newLineChar << indentSpaces << "]";
}

void Node::exportJsonObject(ostream& stream, bool formatted, size_t indent, const String& firstElement,
                            const String& betweenElements, const String& newLineChar,
                            const String& indentSpaces) const
{
    stream << "{";
    if (is(Type::Object))
    {
        bool first = true;
        if (!m_attributes.empty())
        {
            stream << "\"attributes\":";
            if (formatted)
            {
                stream << " { ";
            }
            else
            {
                stream << "{";
            }

            for (const auto&[name, value]: m_attributes)
            {
                if (first)
                {
                    first = false;
                    stream << firstElement;
                }
                else
                {
                    stream << betweenElements;
                }

                stream << "\"" << name << "\":";
                if (formatted)
                {
                    stream << " ";
                }

                if (isInteger(value) || isFloat(value) || isBoolean(value))
                {
                    stream << value;
                }
                else
                {
                    stream << "\"" << value << "\"";
                }
            }

            if (formatted)
            {
                stream << " ";
            }
            stream << "}";

            if (m_nodes.size())
            {
                stream << ",";
            }

            if (formatted)
            {
                stream << " ";
            }
        }

        first = true;
        for (auto& node: m_nodes)
        {
            if (first)
            {
                first = false;
                stream << firstElement;
            }
            else
            {
                stream << betweenElements;
            }
            stream << "\"" << node->name() << "\":";
            if (formatted)
            {
                stream << " ";
            }
            node->exportJsonValueTo(stream, formatted, indent + 2);
        }
    }
    stream << newLineChar << indentSpaces << "}";
}

void Node::exportJson(sptk::Buffer& json, bool formatted) const
{
    stringstream stream;
    exportJsonValueTo(stream, formatted, 0);
    json.set(stream.str());
}
