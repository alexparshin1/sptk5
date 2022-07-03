/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#include <iosfwd>
#include <sptk5/String.h>
#include <sptk5/cutils>
#include <sptk5/xdoc/ExportJSON.h>

#ifdef USE_GTEST
#include <gtest/gtest.h>
#endif

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
    Formatting formatting;

    if (formatted && (node->type() == Node::Type::Array || node->type() == Node::Type::Object))
    {
        if (indent)
        {
            formatting.indentSpaces = string(indent, ' ');
        }
        formatting.newLineChar = "\n";
        formatting.firstElement = "\n  " + formatting.indentSpaces;
        formatting.betweenElements = ",\n  " + formatting.indentSpaces;
    }

    string spacing = formatted ? " " : "";

    bool isValue = node->nodes().empty();

    if (isValue && !node->attributes().empty())
    {
        stream << "{" << spacing;
        exportNodeAttributes(node, stream, formatted, formatting.firstElement);
        stream << "\"value\":" << spacing;
    }

    auto saveFlags = stream.flags();

    double dNumber {0};
    int64_t iNumber {0};
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
        case Node::Type::CData:
            stream << "\"" << jsonEscape(node->getValue().asString()) << "\"";
            break;

        case Node::Type::Boolean:
            stream << (node->getValue().asBool() ? "true" : "false");
            break;

        case Node::Type::Array:
            exportJsonArray(node, stream, formatted, indent, formatting);
            break;

        case Node::Type::Object:
            exportJsonObject(node, stream, formatted, indent, formatting);
            break;

        default:
            stream << "null";
            break;
    }

    if (isValue && !node->attributes().empty())
    {
        stream << spacing << "}";
    }

    stream.flags(saveFlags);
}

void ExportJSON::exportJsonArray(const Node* node, std::ostream& stream, bool formatted, size_t indent,
                                 const Formatting& formatting)
{
    stream << "[";
    if (node->type() == Node::Type::Array)
    {
        bool first = true;
        const auto& array = node->nodes();
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
                stream << formatting.firstElement;
            }
            else
            {
                stream << formatting.betweenElements;
            }
            exportJsonValueTo(element.get(), stream, formatted, indent + 2);
        }
    }
    stream << formatting.newLineChar << formatting.indentSpaces << "]";
}

void ExportJSON::exportJsonObject(const Node* node, std::ostream& stream, bool formatted, size_t indent,
                                  const Formatting& formatting)
{
    stream << "{";
    if (node->type() == Node::Type::Object)
    {
        exportNodeAttributes(node, stream, formatted, formatting.firstElement);

        string spacing = formatted ? " " : "";

        bool first = true;
        for (const auto& anode: node->nodes())
        {
            if (first)
            {
                first = false;
                stream << formatting.firstElement;
            }
            else
            {
                stream << formatting.betweenElements;
            }

            stream << "\"" << anode->name() << "\":" << spacing;

            exportJsonValueTo(anode.get(), stream, formatted, indent + 2);
        }
    }
    stream << formatting.newLineChar << formatting.indentSpaces << "}";
}

void ExportJSON::exportNodeAttributes(const Node* node, ostream& stream, bool formatted, const String& firstElement)
{
    bool first1 = true;
    String spacing = formatted ? " " : "";
    if (!node->attributes().empty())
    {
        stream << firstElement << "\"attributes\":" << spacing << "{";

        for (const auto& [name, value]: node->attributes())
        {
            if (first1)
            {
                first1 = false;
                stream << spacing;
            }
            else
            {
                stream << "," << spacing;
            }

            stream << "\"" << name << "\":" << spacing;

            if (isInteger(value) || isFloat(value) || isBoolean(value))
            {
                stream << value;
            }
            else
            {
                stream << "\"" << value << "\"";
            }
        }

        stream << spacing << "}";

        if (!node->nodes().empty() || !node->attributes().empty())
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

#ifdef USE_GTEST

TEST(SPTK_XDocument, xmlToJson)
{
    xdoc::Document document;
    Buffer buffer;

    buffer.loadFromFile("data/menu.xml");

    document.load(buffer, false);
    document.exportTo(xdoc::DataFormat::JSON, buffer, true);

    //COUT(buffer << endl)
}

#endif
