/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-03-31                                             ║
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

#include <sptk5/String.h>
#include <sptk5/cutils>
#include <sptk5/xdoc/ExportJSON.h>

using namespace std;
using namespace sptk;
using namespace sptk::xdoc;

namespace {
String jsonEscape(const String& text)
{
    static constexpr char hexDigits[] = "0123456789abcdef";
    const auto            len = text.size();

    // Find the first character that needs escaping.
    size_t i = 0;
    for (; i < len; ++i)
    {
        if (const auto ch = static_cast<unsigned char>(text[i]);
            ch < 0x20 || ch == '"' || ch == '\\')
        {
            break;
        }
    }

    if (i == len)
    {
        return text; // nothing to escape — return original without copying
    }

    String result;
    result.reserve(len + (len >> 2));
    result.append(text, 0, i); // copy the clean prefix in one shot

    auto runStart = i;
    for (; i < len; ++i)
    {
        const auto  ch = static_cast<unsigned char>(text[i]);
        const char* esc = nullptr;
        size_t      escLen = 0;
        char        buf[6];

        switch (ch)
        {
            case '"':
                esc = "\\\"";
                escLen = 2;
                break;
            case '\\':
                esc = "\\\\";
                escLen = 2;
                break;
            case '\b':
                esc = "\\b";
                escLen = 2;
                break;
            case '\f':
                esc = "\\f";
                escLen = 2;
                break;
            case '\n':
                esc = "\\n";
                escLen = 2;
                break;
            case '\r':
                esc = "\\r";
                escLen = 2;
                break;
            case '\t':
                esc = "\\t";
                escLen = 2;
                break;
            default:
                if (ch < 0x20)
                {
                    buf[0] = '\\';
                    buf[1] = 'u';
                    buf[2] = '0';
                    buf[3] = '0';
                    buf[4] = hexDigits[ch >> 4];
                    buf[5] = hexDigits[ch & 0x0F];
                    esc = buf;
                    escLen = 6;
                }
                break;
        }

        if (escLen > 0)
        {
            result.append(text, runStart, i - runStart); // flush clean run
            result.append(esc, escLen);
            runStart = i + 1;
        }
    }

    result.append(text, runStart, len - runStart); // flush final clean run
    return result;
}
} // namespace

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

    const string_view spacing = formatted ? " " : "";

    const auto isValue = node->nodes().empty();

    if (isValue && !node->attributes().empty())
    {
        stream << "{" << spacing;
        exportNodeAttributes(node, stream, formatted, formatting.firstElement);
        stream << "\"value\":" << spacing;
    }

    double  dNumber;
    int64_t iNumber;
    switch (node->type())
    {
        case Node::Type::Number:
            iNumber = node->getValue().asInt64();
            dNumber = node->getValue().asFloat();
            if (static_cast<double>(iNumber) == dNumber)
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
            if (isValue)
            {
                if (const auto value = node->getValue().asString();
                    !value.empty())
                {
                    using enum VariantDataType;
                    static const set escapeTypes {VAR_STRING, VAR_TEXT, VAR_DATE, VAR_DATE_TIME};
                    if (escapeTypes.contains(node->getValue().dataType()))
                    {
                        stream << "\"" << jsonEscape(value) << "\"";
                    }
                    else
                    {
                        stream << value;
                    }
                }
                else
                {
                    stream << "{}";
                }
            }
            else
            {
                exportJsonObject(node, stream, formatted, indent, formatting);
            }
            break;

        default:
            stream << "null";
            break;
    }

    if (isValue && !node->attributes().empty())
    {
        stream << spacing << "}";
    }
}

void ExportJSON::exportJsonArray(const Node* node, std::ostream& stream, bool formatted, size_t indent,
                                 const Formatting& formatting)
{
    stream << "[";
    if (node->type() == Node::Type::Array)
    {
        auto        first = true;
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

        const string_view spacing = formatted ? " " : "";

        auto first = true;
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

            stream << "\"" << anode->getQualifiedName() << "\":" << spacing;

            exportJsonValueTo(anode.get(), stream, formatted, indent + 2);
        }
    }
    stream << formatting.newLineChar << formatting.indentSpaces << "}";
}

void ExportJSON::exportNodeAttributes(const Node* node, ostream& stream, bool formatted, const String& firstElement)
{
    const string_view spacing = formatted ? " " : "";

    if (!node->attributes().empty())
    {
        stream << firstElement << "\"attributes\":" << spacing << "{";

        auto first1 = true;
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

void ExportJSON::exportToJSON(const Node* node, Buffer& json, bool formatted)
{
    stringstream stream;
    exportJsonValueTo(node, stream, formatted, 0);
    json.set(stream.str());
}
