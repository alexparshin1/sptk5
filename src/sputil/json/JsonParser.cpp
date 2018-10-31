/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonParser.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 16 2013                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/json/JsonParser.h>

using namespace std;
using namespace sptk;
using namespace sptk::json;

namespace sptk {
    namespace json {
        void skipSpaces(const char* json, const char*& readPosition);
        string readJsonString(const char* json, const char*& readPosition);
        string readJsonName(const char* json, const char*& readPosition);
        void readJsonNull(const char* json, const char*& readPosition);
        bool readJsonBoolean(const char* json, const char*& readPosition);
        double readJsonNumber(const char* json, const char*& readPosition);
        void readArrayData(Element* parent, const char* json, const char*& readPosition);
        void readObjectData(Element* parent, const char* json, const char*& readPosition);
    }
}

#define ERROR_CONTEXT_CHARS 65

void throwError(const string& message, const char* json, size_t position)
{
    stringstream error;
    error << message;
    if (position > 0) {
        error << ", in position " << position;
        char context[ERROR_CONTEXT_CHARS];
        const char* contextStart = json + position - ERROR_CONTEXT_CHARS / 2;
        if (contextStart < json)
            contextStart = json;
        size_t pretextLen = json + position - contextStart;
        strncpy(context, contextStart, pretextLen);
        context[pretextLen] = 0;
        error << " in context: '" << context << ">" << json[position] << "<";
        strncpy(context, json + position + 1, ERROR_CONTEXT_CHARS / 2);
        error << context << "'";
    }
    else if ((int)position < 0)
        error << ", after position " << -int(position);
    throw Exception(error.str());
}

void throwUnexpectedCharacterError(char character, char expected, const char* json, size_t position)
{
    stringstream msg;
    msg << "Unexpected character '" << character << "'";
    if (expected != 0)
        msg << " while expected '" << expected << "'";
    throwError(msg.str(), json, position);
}

void Parser::parse(Element& jsonElement, const string& jsonStr)
{
    const char* json = jsonStr.c_str();
    const char* pos = json;
    skipSpaces(json, pos);

    if (jsonElement.m_type != JDT_NULL)
        throwError("Can't execute on non-null JSON element", json, 0);

    switch (*pos) {
        case '{':
            jsonElement.m_type = JDT_OBJECT;
            jsonElement.m_data.m_object = new ObjectData(jsonElement.getDocument(), &jsonElement);
            readObjectData(&jsonElement, json, pos);
            break;
        case '[':
            jsonElement.m_type = JDT_ARRAY;
            jsonElement.m_data.m_array = new ArrayData(jsonElement.getDocument(), &jsonElement);
            readArrayData(&jsonElement, json, pos);
            break;
        default:
            throwUnexpectedCharacterError(*pos, 0, json, pos - json);
            break;
    }
}

namespace sptk { namespace json {

inline void skipSpaces(const char* json, const char*& readPosition)
{
    while ((unsigned char) *readPosition <= 32) {
        if (*readPosition == 0)
            throwError("Premature end of data", json, readPosition - json);
        readPosition++;
    }
}

string readJsonString(const char* json, const char*& readPosition)
{
    const char* pos = readPosition + 1;
    while (true) {
        pos = strpbrk(pos, "\\\"");
        if (pos == nullptr)
            throwError("Premature end of data, expecting '\"'", json, readPosition - json);
        else {
            char ch = *pos;
            if (ch == '"')
                break;
            if (ch == '\\')
                pos++;
            pos++;
        }
    }
    string str = Element::decode(string(readPosition + 1, pos - readPosition - 1));

    readPosition = pos + 1;

    skipSpaces(json, readPosition);

    return str;
}

string readJsonName(const char* json, const char*& readPosition)
{
    if (*readPosition != '"')
        throwUnexpectedCharacterError(*readPosition, '"', json, readPosition - json);
    string name = readJsonString(json, readPosition);
    if (*readPosition != ':')
        throwUnexpectedCharacterError(*readPosition, ':', json, readPosition - json);
    readPosition++;
    skipSpaces(json, readPosition);
    return name;
}

double readJsonNumber(const char* json, const char*& readPosition)
{
    char* pos;
    errno = 0;
    double value = strtod(readPosition, &pos);
    if (errno != 0)
        throwError("Invalid value", json, readPosition - json);
    readPosition = pos;
    skipSpaces(json, readPosition);
    return value;
}

bool readJsonBoolean(const char* json, const char*& readPosition)
{
    const char* pos = strchr(readPosition + 1, 'e');
    if (pos == nullptr)
        throwError("Premature end of data, expecting boolean value", json, readPosition - json);
    pos++;
    bool result = false;
    if (strncmp(readPosition, "true", 4) == 0)
        result = true;
    else if (strncmp(readPosition, "false", 4) == 0)
        result = false;
    else
        throwError("Unexpected value, expecting boolean", json, readPosition - json);
    readPosition = pos;
    skipSpaces(json, readPosition);
    return result;
}

void readJsonNull(const char* json, const char*& readPosition)
{
    if (strncmp(readPosition, "null", 4) != 0)
        throwError("Unexpected value, expecting 'null'", json, readPosition - json);
    readPosition += 4;
    if (*readPosition == ',')
        readPosition++;
    skipSpaces(json, readPosition);
}

void readArrayData(Element* parent, const char* json, const char*& readPosition)
{
    if (*readPosition != '[')
        throwUnexpectedCharacterError(*readPosition, '[', json, readPosition - json);

    readPosition++;

    while (*readPosition != ']') {
        skipSpaces(json, readPosition);

        char firstChar = *readPosition;
        if (isdigit(firstChar))
            firstChar = '0';

        switch (firstChar) {
            case ']':
                // Close bracket
                break;

            case '[':
            {
                auto* jsonArrayElement = parent->push_array();
                readArrayData(jsonArrayElement, json, readPosition);
            }
            break;

            case '{':
            {
                auto* jsonObjectElement = parent->push_object();
                readObjectData(jsonObjectElement, json, readPosition);
            }
            break;

            case '0':
            case '-':
            {
                // Number
                double number = readJsonNumber(json, readPosition);
                parent->push_back(number);
            }
            break;

            case 't':
            case 'f':
            {
                // Boolean
                bool value = readJsonBoolean(json, readPosition);
                parent->push_back(value);
            }
            break;

            case 'n':
            {
                // Null
                readJsonNull(json, readPosition);
                parent->push_back();
            }
            break;

            case '"':
            {
                // String
                string str = readJsonString(json, readPosition);
                parent->push_back(str);
            }
            break;

            case ',':
                readPosition++;
                skipSpaces(json, readPosition);
                break;

            default:
                throwUnexpectedCharacterError(*readPosition, 0, json, readPosition - json);
                break;
        }
    }
    readPosition++;
}

void readObjectData(Element* parent, const char* json, const char*& readPosition)
{
    if (*readPosition != '{')
        throwUnexpectedCharacterError(*readPosition, '{', json, readPosition - json);

    readPosition++;

    while (*readPosition != '}') {
        skipSpaces(json, readPosition);
        if (*readPosition == ',') {
            readPosition++;
            continue;
        }

        if (*readPosition == '}')
            continue;

        string elementName = readJsonName(json, readPosition);

        char firstChar = *readPosition;
        if (isdigit(firstChar))
            firstChar = '0';

        switch (firstChar) {
            case '}':
                // Close bracket
                break;

            case '[':
            {
                auto jsonArrayElement = parent->set_array(elementName);
                readArrayData(jsonArrayElement, json, readPosition);
            }
            break;

            case '{':
            {
                auto jsonObjectElement = parent->set_object(elementName);
                readObjectData(jsonObjectElement, json, readPosition);
            }
            break;

            case '0':
            case '-':
            {
                // Number
                double number = readJsonNumber(json, readPosition);
                parent->set(elementName, number);
            }
            break;

            case 't':
            case 'f':
            {
                // Boolean
                bool value = readJsonBoolean(json, readPosition);
                parent->set(elementName, value);
            }
            break;

            case 'n':
            {
                // Null
                readJsonNull(json, readPosition);
                parent->set(elementName);
            }
            break;

            case '"':
            {
                // String
                string str = readJsonString(json, readPosition);
                parent->set(elementName, str);
            }
            break;

            default:
                throwUnexpectedCharacterError(*readPosition, 0, json, readPosition - json);
                break;
        }
    }
    readPosition++;
}

}}
