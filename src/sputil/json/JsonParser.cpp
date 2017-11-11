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
#include <string.h>
#include <iostream>

using namespace std;
using namespace sptk;
using namespace sptk::json;

namespace sptk { namespace json {
    void error(const string& message, size_t position = 0);
    void skipSpaces(const std::string& json, size_t& readPosition);
    std::string readJsonString(const std::string& json, size_t& readPosition);
    std::string readJsonName(const std::string& json, size_t& readPosition);
    void readJsonNull(const std::string& json, size_t& readPosition);
    bool readJsonBoolean(const std::string& json, size_t& readPosition);
    double readJsonNumber(const std::string& json, size_t& readPosition);
    void readArrayData(Element* parent, const std::string& json, size_t& readPosition);
    void readObjectData(Element* parent, const std::string& json, size_t& readPosition);
} }

Parser::Parser()
{}

void Parser::parse(Element& jsonElement, const string& json)
{
    size_t pos = 0;
    skipSpaces(json, pos);

    if (pos == string::npos)
        error("Can't find JSON", 0);

    if (jsonElement.m_type != JDT_NULL)
        error("Can't execute on non-null JSON element", 0);

    switch (json[pos]) {
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
            error("Unexpected character in position", pos);
            break;
    }
}

namespace sptk { namespace json {

void error(const string& message, size_t position)
{
    stringstream error;
    error << message;
    if (position > 0)
        error << ", in position 0x" << hex << position;
    else if (int(position) < 0)
        error << ", after position 0x" << hex << -int(position);
    throw runtime_error(error.str());
}

void skipSpaces(const string& json, size_t& readPosition)
{
    size_t pos = json.find_first_not_of(" \n\t\r", readPosition);
    if (pos == string::npos)
        error("Missing data", readPosition);
    readPosition = pos;
}

string readJsonString(const string& json, size_t& readPosition)
{
    size_t pos = readPosition + 1;
    while (true) {
        pos = json.find_first_of("\\\"", pos);
        if (pos == string::npos)
            error("Unexpected character", readPosition);
        char ch = json[pos];
        if (ch == '"')
            break;
        if (ch == '\\')
            pos++;
        pos++;
    }
    //if (readPosition == 188020)
    //    cout << endl;
    string str = Element::decode(json.substr(readPosition + 1, pos - readPosition - 1));

    readPosition = pos + 1;
    skipSpaces(json, readPosition);

    return str;
}

string readJsonName(const string& json, size_t& readPosition)
{
    if (json[readPosition] != '"')
        error("Unexpected character, expecting '\"'", readPosition);
    string name = readJsonString(json, readPosition);
    if (json[readPosition] != ':')
        error("Unexpected character, expecting ':'", readPosition);
    readPosition++;
    skipSpaces(json, readPosition);
    return name;
}

double readJsonNumber(const string& json, size_t& readPosition)
{
    size_t pos = json.find_first_not_of("-+0123456789.Ee", readPosition + 1);
    if (pos == string::npos)
        error("Unexpected character", readPosition);
    string number = json.substr(readPosition, pos - readPosition);
    readPosition = pos;
    skipSpaces(json, readPosition);
    return atof(number.c_str());
}

bool readJsonBoolean(const string& json, size_t& readPosition)
{
    size_t pos = json.find_first_of("e", readPosition + 1);
    if (pos == string::npos)
        error("Unexpected character", readPosition);
    pos++;
    string value = json.substr(readPosition, pos - readPosition);
    bool result = false;
    if (value == "true")
        result = true;
    else if (value == "false")
        result = false;
    else
        error("Unexpected value", readPosition);
    readPosition = pos;
    skipSpaces(json, readPosition);
    return result;
}

void readJsonNull(const string& json, size_t& readPosition)
{
    size_t pos = json.find_first_not_of("nul", readPosition + 1);
    if (pos == string::npos)
        error("Unexpected character", readPosition);
    string value = json.substr(readPosition, pos - readPosition);
    if (value != "null")
        error("Unexpected value", readPosition);
    readPosition = pos;
    skipSpaces(json, readPosition);
}

void readArrayData(Element* parent, const string& json, size_t& readPosition)
{
    if (json[readPosition] != '[')
        error("Unexpected character", readPosition);

    readPosition++;

    while (json[readPosition] != ']') {
        skipSpaces(json, readPosition);

        char firstChar = json[readPosition];
        if (isdigit(firstChar))
            firstChar = '0';

        switch (firstChar) {
            case ']':
                // Close bracket
                break;

            case '[':
            {
                Element* jsonArrayElement = new Element(new ArrayData);
                readArrayData(jsonArrayElement, json, readPosition);
                parent->add(jsonArrayElement);
            }
            break;

            case '{':
            {
                Element* jsonObjectElement = new Element(new ObjectData);
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
                error("Unexpected character", readPosition);
                break;
        }
    }
    readPosition++;
}

void readObjectData(Element* parent, const string& json, size_t& readPosition)
{
    if (json[readPosition] != '{')
        error("Unexpected character", readPosition);

    readPosition++;

    while (json[readPosition] != '}') {
        skipSpaces(json, readPosition);
        if (json[readPosition] == ',') {
            readPosition++;
            continue;
        }

        if (json[readPosition] == '}')
            continue;

        string elementName = readJsonName(json, readPosition);

        char firstChar = json[readPosition];
        if (isdigit(firstChar))
            firstChar = '0';

        switch (firstChar) {
            case '}':
                // Close bracket
                break;

            case '[':
            {
                Element* jsonArrayElement = new Element(new ArrayData);
                readArrayData(jsonArrayElement, json, readPosition);
                parent->add(elementName, jsonArrayElement);
            }
            break;

            case '{':
            {
                Element* jsonObjectElement = new Element(new ObjectData);
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
                error("Unexpected character", readPosition);
                break;
        }
    }
    readPosition++;
}

}}
