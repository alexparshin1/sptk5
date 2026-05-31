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

#include <sptk5/Buffer.h>
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

void ExportJSON::exportJsonValueTo(const Node* node, Buffer& json, const bool formatted,
                                   const size_t indent)
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

    const auto isValue = node->nodes().empty();

    if (isValue && !node->attributes().empty())
    {
        json.append('{');
        if (formatted)
            json.append(' ');
        exportNodeAttributes(node, json, formatted, formatting.firstElement);
        json.append("\"value\":", 8);
        if (formatted)
            json.append(' ');
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
                json.append(22, "{}", iNumber);
            }
            else
            {
                const auto s = node->getValue().asString();
                json.append(s);
            }
            break;

        case Node::Type::Text:
        case Node::Type::CData:
        {
            const auto escaped = jsonEscape(node->getValue().asString());
            json.append('"');
            json.append(escaped);
            json.append('"');
            break;
        }

        case Node::Type::Boolean:
            if (node->getValue().asBool())
                json.append("true", 4);
            else
                json.append("false", 5);
            break;

        case Node::Type::Array:
            exportJsonArray(node, json, formatted, indent, formatting);
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
                        const auto escaped = jsonEscape(value);
                        json.append('"');
                        json.append(escaped);
                        json.append('"');
                    }
                    else
                    {
                        json.append(value);
                    }
                }
                else
                {
                    json.append("{}", 2);
                }
            }
            else
            {
                exportJsonObject(node, json, formatted, indent, formatting);
            }
            break;

        default:
            json.append("null", 4);
            break;
    }

    if (isValue && !node->attributes().empty())
    {
        if (formatted)
            json.append(' ');
        json.append('}');
    }
}

void ExportJSON::exportJsonArray(const Node* node, Buffer& json, const bool formatted, const size_t indent,
                                 const Formatting& formatting)
{
    json.append('[');
    if (node->type() == Node::Type::Array)
    {
        const auto& array = node->nodes();
        if (array.empty())
        {
            json.append(']');
            return;
        }
        auto first = true;
        for (const auto& element: array)
        {
            if (first)
            {
                first = false;
                json.append(formatting.firstElement);
            }
            else
            {
                json.append(formatting.betweenElements);
            }
            exportJsonValueTo(element.get(), json, formatted, indent + 2);
        }
    }
    json.append(formatting.newLineChar);
    json.append(formatting.indentSpaces);
    json.append(']');
}

void ExportJSON::exportJsonObject(const Node* node, Buffer& json, const bool formatted, const size_t indent,
                                  const Formatting& formatting)
{
    json.append('{');
    if (node->type() == Node::Type::Object)
    {
        exportNodeAttributes(node, json, formatted, formatting.firstElement);

        auto first = true;
        for (const auto& anode: node->nodes())
        {
            if (first)
            {
                first = false;
                json.append(formatting.firstElement);
            }
            else
            {
                json.append(formatting.betweenElements);
            }

            json.append('"');
            json.append(anode->getQualifiedName());
            json.append("\":", 2);
            if (formatted)
                json.append(' ');

            exportJsonValueTo(anode.get(), json, formatted, indent + 2);
        }
    }
    json.append(formatting.newLineChar);
    json.append(formatting.indentSpaces);
    json.append('}');
}

void ExportJSON::exportNodeAttributes(const Node* node, Buffer& json, const bool formatted, const String& firstElement)
{
    if (!node->attributes().empty())
    {
        json.append(firstElement);
        json.append("\"attributes\":", 13);
        if (formatted)
            json.append(' ');
        json.append('{');

        auto first1 = true;
        for (const auto& [name, value]: node->attributes())
        {
            if (first1)
            {
                first1 = false;
                if (formatted)
                    json.append(' ');
            }
            else
            {
                json.append(',');
                if (formatted)
                    json.append(' ');
            }

            json.append('"');
            json.append(name);
            json.append("\":", 2);
            if (formatted)
                json.append(' ');

            if (isInteger(value) || isFloat(value) || isBoolean(value))
            {
                json.append(value);
            }
            else
            {
                json.append('"');
                json.append(value);
                json.append('"');
            }
        }

        if (formatted)
            json.append(' ');
        json.append('}');

        if (!node->nodes().empty() || !node->attributes().empty())
        {
            json.append(',');
        }

        if (formatted)
        {
            json.append(' ');
        }
    }
}

void ExportJSON::exportToJSON(const Node* node, Buffer& json, const bool formatted)
{
    json.reset();
    exportJsonValueTo(node, json, formatted, 0);
}
