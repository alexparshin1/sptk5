/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonParser.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 16 2013                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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
#include <sstream>
#include <cstring>
#include <iostream>

using namespace std;
using namespace sptk;
using namespace sptk::json;

namespace sptk { namespace json {
    void throwError(const string& message, size_t position = 0);
    void skipSpaces(const char* json, const char*& readPosition);
    std::string readJsonString(const char* json, const char*& readPosition);
    std::string readJsonName(const char* json, const char*& readPosition);
    void readJsonNull(const char* json, const char*& readPosition);
    bool readJsonBoolean(const char* json, const char*& readPosition);
    double readJsonNumber(const char* json, const char*& readPosition);
    void readArrayData(Element* parent, const char* json, const char*& readPosition);
    void readObjectData(Element* parent, const char* json, const char*& readPosition);
} }

void Parser::parse(Element& jsonElement, const string& jsonStr)
{
    const char* json = jsonStr.c_str();
    const char* pos = json;
    skipSpaces(json, pos);

    if (pos == nullptr)
        throwError("Can't find JSON", 0);

    if (jsonElement.m_type != JDT_NULL)
        throwError("Can't execute on non-null JSON element", 0);

    switch (*pos) {
        case '{':
            jsonElement.m_type = JDT_OBJECT;
            jsonElement.m_data.m_object = new ObjectData(&jsonElement);
            readObjectData(&jsonElement, json, pos);
            break;
        case '[':
            jsonElement.m_type = JDT_ARRAY;
            jsonElement.m_data.m_array = new ArrayData(&jsonElement);
            readArrayData(&jsonElement, json, pos);
            break;
        default:
            {
                stringstream msg;
                msg << "Unexpected character '" << *pos << "' in position";
                throwError(msg.str(), pos - json);
            }
            break;
    }
}

namespace sptk { namespace json {

void throwError(const string& message, size_t position)
{
    stringstream error;
    error << message;
    if (position > 0)
        error << ", in position " << position;
    else if (int(position) < 0)
        error << ", after position " << -int(position);
    throw runtime_error(error.str());
}

inline void skipSpaces(const char* json, const char*& readPosition)
{
    while ((unsigned char) *readPosition <= 32) {
        if (*readPosition == 0)
            throwError("Premature end of data", readPosition - json);
        readPosition++;
    }
}

string readJsonString(const char* json, const char*& readPosition)
{
    const char* pos = readPosition + 1;
    while (true) {
        pos = strpbrk(pos, "\\\"");
        if (pos == nullptr)
            throwError("Unexpected character", readPosition - json);
        char ch = *pos;
        if (ch == '"')
            break;
        if (ch == '\\')
            pos++;
        pos++;
    }
    string str = Element::decode(string(readPosition + 1, pos - readPosition - 1));

    readPosition = pos + 1;

    skipSpaces(json, readPosition);

    return str;
}

string readJsonName(const char* json, const char*& readPosition)
{
    if (*readPosition != '"')
        throwError("Unexpected character, expecting '\"'", readPosition - json);
    string name = readJsonString(json, readPosition);
    if (*readPosition != ':')
        throwError("Unexpected character, expecting ':'", readPosition - json);
    readPosition++;
    skipSpaces(json, readPosition);
    return name;
}

double readJsonNumber(const char* json, const char*& readPosition)
{
    char* pos;
    double value = strtod(readPosition, &pos);
    if (errno != 0)
        throwError("Invalid value", readPosition - json);
    readPosition = pos;
    skipSpaces(json, readPosition);
    return value;
}

bool readJsonBoolean(const char* json, const char*& readPosition)
{
    const char* pos = strchr(readPosition + 1, 'e');
    if (pos == nullptr)
        throwError("Unexpected character", readPosition - json);
    pos++;
    string value(readPosition, pos - readPosition);
    bool result = false;
    if (strncmp(readPosition, "true", 4) == 0)
        result = true;
    else if (strncmp(readPosition, "false", 4) == 0)
        result = false;
    else
        throwError("Unexpected value", readPosition - json);
    readPosition = pos;
    skipSpaces(json, readPosition);
    return result;
}

void readJsonNull(const char* json, const char*& readPosition)
{
    if (strncmp(readPosition, "null", 4) != 0)
        throwError("Unexpected value", readPosition - json);
    readPosition += 5;
    skipSpaces(json, readPosition);
}

void readArrayData(Element* parent, const char* json, const char*& readPosition)
{
    if (*readPosition != '[')
        throwError("Unexpected character", readPosition - json);

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
                auto jsonArrayElement = new Element(new ArrayData);
                readArrayData(jsonArrayElement, json, readPosition);
                parent->add(jsonArrayElement);
            }
            break;

            case '{':
            {
                auto jsonObjectElement = new Element(new ObjectData);
                readObjectData(jsonObjectElement, json, readPosition);
                parent->add(jsonObjectElement);
            }
            break;

            case '0':
            case '-':
            {
                // Number
                double number = readJsonNumber(json, readPosition);
                parent->add(new Element(number));
            }
            break;

            case 't':
            case 'f':
            {
                // Boolean
                bool value = readJsonBoolean(json, readPosition);
                parent->add(new Element(value));
            }
            break;

            case 'n':
            {
                // Null
                readJsonNull(json, readPosition);
                parent->add(new Element());
            }
            break;

            case '"':
            {
                // String
                string str = readJsonString(json, readPosition);
                parent->add(new Element(str));
            }
            break;

            case ',':
                readPosition++;
                skipSpaces(json, readPosition);
                break;

            default:
                throwError("Unexpected character", readPosition - json);
                break;
        }
    }
    readPosition++;
}

void readObjectData(Element* parent, const char* json, const char*& readPosition)
{
    if (*readPosition != '{')
        throwError("Unexpected character", readPosition - json);

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
                auto jsonArrayElement = new Element(new ArrayData);
                readArrayData(jsonArrayElement, json, readPosition);
                parent->add(elementName, jsonArrayElement);
            }
            break;

            case '{':
            {
                auto jsonObjectElement = new Element(new ObjectData);
                readObjectData(jsonObjectElement, json, readPosition);
                parent->add(elementName, jsonObjectElement);
            }
            break;

            case '0':
            case '-':
            {
                // Number
                double number = readJsonNumber(json, readPosition);
                parent->add(elementName, new Element(number));
            }
            break;

            case 't':
            case 'f':
            {
                // Boolean
                bool value = readJsonBoolean(json, readPosition);
                parent->add(elementName, new Element(value));
            }
            break;

            case 'n':
            {
                // Null
                readJsonNull(json, readPosition);
                parent->add(elementName, new Element());
            }
            break;

            case '"':
            {
                // String
                string str = readJsonString(json, readPosition);
                parent->add(elementName, new Element(str));
            }
            break;

            default:
                throwError("Unexpected character", readPosition - json);
                break;
        }
    }
    readPosition++;
}

}}
