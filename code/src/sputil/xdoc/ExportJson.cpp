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

#include <sptk5/xdoc/ExportJSON.h>
#include <sptk5/RegularExpression.h>
#include <iosfwd>
#include <sptk5/String.h>
#include <sptk5/cutils>

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

void ExportJSON::exportJsonValueTo(const Node* node, ostream& stream, bool formatted,
                                   size_t indent)
{
    String indentSpaces;
    String newLineChar;
    String firstElement;
    String betweenElements(",");

    if (formatted && (node->is(Node::Type::Array) || node->is(Node::Type::Object)))
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
    switch (node->type())
    {
        case Node::Type::Number:
            iNumber = node->getValue().asInt64();
            dNumber = node->getValue().asFloat();
            if (double(iNumber) == dNumber)
            {
                stream << fixed << iNumber;
            }
            else
            {
                stream << node->getValue().asString();
            }
            break;

        case Node::Type::Text:
            stream << "\"" << jsonEscape(node->getValue().asString()) << "\"";
            break;

        case Node::Type::CData:
            stream << "\"" << jsonEscape(node->getValue().asString()) << "\"";
            break;

        case Node::Type::Boolean:
            stream << (node->getValue().asBool() ? "true" : "false");
            break;

        case Node::Type::Array:
            exportJsonArray(node, stream, formatted, indent, firstElement, betweenElements, newLineChar, indentSpaces);
            break;

        case Node::Type::Object:
            exportJsonObject(node, stream, formatted, indent, firstElement, betweenElements, newLineChar, indentSpaces);
            break;

        default:
            stream << "null";
            break;
    }
    stream.flags(saveFlags);
}

void ExportJSON::exportJsonArray(const Node* node, ostream& stream, bool formatted, size_t indent,
                                 const String& firstElement,
                                 const String& betweenElements, const String& newLineChar,
                                 const String& indentSpaces)
{
    stream << "[";
    if (node->is(Node::Type::Array))
    {
        bool first = true;
        const auto& array = node->getArray();
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
            exportJsonValueTo(element.get(), stream, formatted, indent + 2);
        }
    }
    stream << newLineChar << indentSpaces << "]";
}

void ExportJSON::exportJsonObject(const Node* node, ostream& stream, bool formatted, size_t indent,
                                  const String& firstElement,
                                  const String& betweenElements, const String& newLineChar,
                                  const String& indentSpaces)
{
    stream << "{";
    if (node->is(Node::Type::Object))
    {
        exportNodeAttributes(node, stream, formatted, firstElement, betweenElements);

        bool first = true;
        for (auto& anode: node->getArray())
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
            stream << "\"" << anode->name() << "\":";
            if (formatted)
            {
                stream << " ";
            }
            exportJsonValueTo(anode.get(), stream, formatted, indent + 2);
        }
    }
    stream << newLineChar << indentSpaces << "}";
}

void ExportJSON::exportNodeAttributes(const Node* node, ostream& stream, bool formatted, const String& firstElement,
                                      const String& betweenElements)
{
    bool first1 = true;
    if (!node->attributes().empty())
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

        for (const auto&[name, value]: node->attributes())
        {
            if (first1)
            {
                first1 = false;
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

        if (node->getArray().size())
        {
            stream << ",";
        }

        if (formatted)
        {
            stream << " ";
        }
    }
}

void ExportJSON::exportToJSON(const Node* node, sptk::Buffer& json, bool formatted)
{
    stringstream stream;
    exportJsonValueTo(node, stream, formatted, 0);
    json.set(stream.str());
}
