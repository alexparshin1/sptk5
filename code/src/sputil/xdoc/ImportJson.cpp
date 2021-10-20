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

#include <sptk5/Printer.h>
#include <sptk5/xdoc/Document.h>
#include <sptk5/xdoc/ExportJSON.h>

using namespace std;
using namespace sptk;
using namespace sptk::xdoc;

namespace sptk::xdoc {
void skipSpaces(const char* json, const char*& readPosition);

String readJsonString(const char* json, const char*& readPosition);

String readJsonName(const char* json, const char*& readPosition);

void readJsonNull(const char* json, const char*& readPosition);

bool readJsonBoolean(const char* json, const char*& readPosition);

double readJsonNumber(const char* json, const char*& readPosition);

void readArrayData(const SNode& parent, const char* json, const char*& readPosition);

void readObjectData(const SNode& parent, const char* json, const char*& readPosition);

String decode(const String& text);
} // namespace sptk::xdoc

static constexpr int ERROR_CONTEXT_CHARS = 65;

[[noreturn]] void throwError(const String& message, const char* json, size_t position)
{
    stringstream error;
    error << message;
    auto jsonLength = strlen(json);
    if (position > 0)
    {
        error << ", in position " << position;
        const char* contextStart = json + position - ERROR_CONTEXT_CHARS / 2;
        if (contextStart < json)
        {
            contextStart = json;
        }
        size_t pretextLen = json + position - contextStart;
        error << " in context: '.." << String(contextStart, pretextLen) << ">" << json[position] << "<";
        size_t afterTextLength = ERROR_CONTEXT_CHARS / 2;
        if (int(position) + afterTextLength > jsonLength)
        {
            afterTextLength = jsonLength - position;
        }
        if (afterTextLength > 0)
        {
            error << String(json + position + 1, afterTextLength);
        }
        error << "'";
    }
    else if ((int) position < 0)
    {
        error << ", after position " << -int(position);
    }
    throw Exception(error.str());
}

[[noreturn]] static void throwUnexpectedCharacterError(char character, char expected, const char* json, size_t position)
{
    stringstream msg;
    msg << "Unexpected character '" << character << "'";
    if (expected != 0)
    {
        msg << " while expected '" << expected << "'";
    }
    throwError(msg.str(), json, position);
}

void Node::importJson(const SNode& jsonElement, const Buffer& jsonStr)
{
    const char* json = jsonStr.c_str();
    const char* pos = json;
    skipSpaces(json, pos);

    switch (*pos)
    {
        case '{':
            jsonElement->type(Node::Type::Object);
            readObjectData(jsonElement, json, pos);
            break;
        case '[':
            jsonElement->type(Node::Type::Array);
            readArrayData(jsonElement, json, pos);
            break;
        default:
            throwUnexpectedCharacterError(*pos, 0, json, pos - json);
    }

    // Check if there is trailing junk data
    while (*pos)
    {
        if ((unsigned char) *pos > ' ')
        {
            throwError("Invalid character(s) after JSON data", json, strlen(json));
        }
        ++pos;
    }
}

namespace sptk::xdoc {

inline void skipSpaces(const char* json, const char*& readPosition)
{
    while ((unsigned char) *readPosition <= ' ')
    {
        if (*readPosition == 0)
        {
            throwError("Premature end of data", json, strlen(json));
        }
        ++readPosition;
    }
}

String readJsonString(const char* json, const char*& readPosition)
{
    const char* pos = readPosition + 1;
    while (true)
    {
        pos = strpbrk(pos, "\\\"");
        if (pos == nullptr)
        {
            throw Exception(R"(Premature end of data, expecting '"')");
        }
        char ch = *pos;
        if (ch == '"')
        {
            break;
        }
        if (ch == '\\')
        {
            ++pos;
        }
        ++pos;
    }
    String str = decode(string(readPosition + 1, pos - readPosition - 1));

    readPosition = pos + 1;

    skipSpaces(json, readPosition);

    return str;
}

String readJsonName(const char* json, const char*& readPosition)
{
    if (*readPosition != '"')
    {
        throwUnexpectedCharacterError(*readPosition, '"', json, readPosition - json);
    }
    String name = readJsonString(json, readPosition);
    if (*readPosition != ':')
    {
        throwUnexpectedCharacterError(*readPosition, ':', json, readPosition - json);
    }
    ++readPosition;
    skipSpaces(json, readPosition);
    return name;
}

double readJsonNumber(const char* json, const char*& readPosition)
{
    char* pos = nullptr;
    errno = 0;
    double value = strtod(readPosition, &pos);
    if (errno != 0)
    {
        throwError("Invalid value", json, readPosition - json);
    }
    readPosition = pos;
    skipSpaces(json, readPosition);
    return value;
}

bool readJsonBoolean(const char* json, const char*& readPosition)
{
    const char* pos = strchr(readPosition + 1, 'e');
    if (pos == nullptr)
    {
        throwError("Premature end of data, expecting boolean value", json, readPosition - json);
    }
    ++pos;
    bool result;
    if (strncmp(readPosition, "true", 4) == 0)
    {
        result = true;
    }
    else if (strncmp(readPosition, "false", 4) == 0)
    {
        result = false;
    }
    else
    {
        throwError("Unexpected value, expecting boolean", json, readPosition - json);
    }
    readPosition = pos;
    skipSpaces(json, readPosition);
    return result;
}

void readJsonNull(const char* json, const char*& readPosition)
{
    if (strncmp(readPosition, "null", 4) != 0)
    {
        throwError("Unexpected value, expecting 'null'", json, readPosition - json);
    }
    readPosition += 4;
    if (*readPosition == ',')
    {
        ++readPosition;
    }
    skipSpaces(json, readPosition);
}

void readArrayData(const SNode& parent, const char* json, const char*& readPosition)
{
    if (*readPosition != '[')
    {
        throwUnexpectedCharacterError(*readPosition, '[', json, readPosition - json);
    }

    ++readPosition;

    while (*readPosition != ']')
    {
        skipSpaces(json, readPosition);

        char firstChar = *readPosition;
        if (isdigit(firstChar))
        {
            firstChar = '0';
        }

        switch (firstChar)
        {
            case ']':
                // Close bracket
                break;

            case '[':

                xdoc::readArrayData(parent->pushNode("", Node::Type::Array), json, readPosition);
                break;

            case '{':
                xdoc::readObjectData(parent->pushNode("", Node::Type::Object), json, readPosition);
                break;

            case '0':
            case '-':
                // Number
                parent->pushValue(readJsonNumber(json, readPosition), Node::Type::Number);
                break;

            case 't':
            case 'f':
                // Boolean
                parent->pushValue(readJsonBoolean(json, readPosition), Node::Type::Boolean);
                break;

            case 'n':
                // Null
                readJsonNull(json, readPosition);
                parent->pushValue(Variant(), Node::Type::Null);
                break;

            case '"':
                // String
                parent->pushValue(readJsonString(json, readPosition), Node::Type::Text);
                break;

            case ',':
                ++readPosition;
                skipSpaces(json, readPosition);
                break;

            default:
                throwUnexpectedCharacterError(*readPosition, 0, json, readPosition - json);
        }
    }
    ++readPosition;
}

void readObjectData(const SNode& parent, const char* json, const char*& readPosition)
{
    if (*readPosition != '{')
    {
        throwUnexpectedCharacterError(*readPosition, '{', json, readPosition - json);
    }

    ++readPosition;

    while (*readPosition != '}')
    {
        skipSpaces(json, readPosition);
        if (*readPosition == ',')
        {
            ++readPosition;
            continue;
        }

        if (*readPosition == '}')
        {
            continue;
        }

        String elementName = readJsonName(json, readPosition);

        char firstChar = *readPosition;
        if (isdigit(firstChar))
        {
            firstChar = '0';
        }

        switch (firstChar)
        {
            case '}':
                // Close bracket
                break;

            case '[':
                xdoc::readArrayData(parent->pushNode(elementName, Node::Type::Array),
                                    json, readPosition);
                break;

            case '{':
                xdoc::readObjectData(parent->pushNode(elementName, Node::Type::Object), json, readPosition);
                break;

            case '0':
            case '-':
                // Number
                parent->pushValue(elementName, readJsonNumber(json, readPosition), Node::Type::Number);
                break;

            case 't':
            case 'f':
                // Boolean
                parent->pushValue(elementName, readJsonBoolean(json, readPosition), Node::Type::Boolean);
                break;

            case 'n':
                // Null
                readJsonNull(json, readPosition);
                parent->pushValue(elementName, Variant(), Node::Type::Null);
                break;

            case '"':
                // String
                parent->pushValue(elementName, readJsonString(json, readPosition), Node::Type::Text);
                break;

            default:
                throwUnexpectedCharacterError(*readPosition, 0, json, readPosition - json);
        }
    }
    ++readPosition;
}

static String codePointToUTF8(unsigned cp)
{
    String result;

    // based on description from http://en.wikipedia.org/wiki/UTF-8

    if (cp <= 0x7f)
    {
        result.resize(1);
        result[0] = static_cast<char>(cp);
    }
    else if (cp <= 0x7FF)
    {
        result.resize(2);
        result[1] = static_cast<char>(0x80 | (0x3f & cp));
        result[0] = static_cast<char>(0xC0 | (0x1f & (cp >> 6)));
    }
    else if (cp <= 0xFFFF)
    {
        result.resize(3);
        result[2] = static_cast<char>(0x80 | (0x3f & cp));
        result[1] = char(0x80 | static_cast<char>((0x3f & (cp >> 6))));
        result[0] = char(0xE0 | static_cast<char>((0xf & (cp >> 12))));
    }
    else if (cp <= 0x10FFFF)
    {
        result.resize(4);
        result[3] = static_cast<char>(0x80 | (0x3f & cp));
        result[2] = static_cast<char>(0x80 | (0x3f & (cp >> 6)));
        result[1] = static_cast<char>(0x80 | (0x3f & (cp >> 12)));
        result[0] = static_cast<char>(0xF0 | (0x7 & (cp >> 18)));
    }

    return result;
}

String decode(const String& text)
{
    String result;
    size_t length = text.length();
    size_t position = 0;
    unsigned ucharCode;

    while (position < length)
    {
        size_t pos = text.find_first_of('\\', position);
        if (pos == string::npos)
        {
            if (position == 0)
            {
                return text;
            }
            result += text.substr(position);
            break;
        }
        if (pos != position)
        {
            result += text.substr(position, pos - position);
        }
        ++pos;
        switch (text[pos])
        {
            case '"':
                result += '"';
                break;
            case '\\':
                result += '\\';
                break;
            case '/':
                result += '/';
                break;
            case 'b':
                result += '\b';
                break;
            case 'f':
                result += '\f';
                break;
            case 'n':
                result += '\n';
                break;
            case 'r':
                result += '\r';
                break;
            case 't':
                result += '\t';
                break;
            case 'u':
                ++pos;
                ucharCode = (unsigned) strtol(text.substr(pos, 4).c_str(), nullptr, 16);
                pos += 3;
                result += codePointToUTF8(ucharCode);
                break;

            default:
                throw Exception("Unknown escape character");
        }
        position = pos + 1;
    }

    return result;
}

} // namespace sptk::xdoc

#ifdef USE_GTEST

static const String testJson(
    R"({"name":"John","age":33,"temperature":33.6,"timestamp":1519005758000,)"
    R"("skills":["C++","Java","Motorbike"],)"
    R"("location":null,)"
    R"("description":"Title: \"Mouse\"\r\nPosition:\t\fManager/Janitor\b",)"
    R"("value":"\\0x05",)"
    R"("title":"\"Mouse\"",)"
    R"("name":"Юстас",)"
    R"("address":{"married":true,"employed":false}})");

static const String testFormattedJson(R"({
  "name": "John",
  "age": 33,
  "temperature": 33.6,
  "timestamp": 1519005758000,
  "skills": [
    "C++",
    "Java",
    "Motorbike"
  ],
  "location": null,
  "description": "Title: \"Mouse\"\r\nPosition:\t\fManager/Janitor\b",
  "value": "\\0x05",
  "title": "\"Mouse\"",
  "name": "Юстас",
  "address": {
    "married": true,
    "employed": false
  }
})");

TEST(SPTK_XDocument, formatJSON)
{
    Buffer input(testJson);
    xdoc::Document document;
    const auto& root = document.root();
    Node::importJson(root, input);

    Buffer output;
    ExportJSON::exportToJSON(root.get(), output, false);

    EXPECT_STREQ(testJson.c_str(), output.c_str());

    ExportJSON::exportToJSON(root.get(), output, true);
    EXPECT_STREQ(testFormattedJson.c_str(), output.c_str());
}

TEST(SPTK_XDocument, importJsonExceptions)
{
    Buffer input("<?xml?>");
    xdoc::Document document;
    const auto& root = document.root();
    EXPECT_THROW(Node::importJson(root, input), Exception);
}

#endif
