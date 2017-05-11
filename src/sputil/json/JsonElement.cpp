/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonElement.cpp - description                          ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 16 2013                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#include <sptk5/json/JsonElement.h>
#include <sptk5/json/JsonArrayData.h>
#include <sstream>
#include <string.h>

using namespace std;
using namespace sptk;
using namespace sptk::json;

const Element Element::emptyElement("");

Element::Element(double value)
: m_parent(NULL), m_type(JDT_NUMBER)
{
    m_data.m_number = value;
}

Element::Element(int value)
: m_parent(NULL), m_type(JDT_NUMBER)
{
    m_data.m_number = value;
}

Element::Element(string value)
: m_parent(NULL), m_type(JDT_STRING)
{
    m_data.m_string = new string(value);
}

Element::Element(const char* value)
: m_parent(NULL), m_type(JDT_STRING)
{
    m_data.m_string = new string(value);
}

Element::Element(bool value)
: m_parent(NULL), m_type(JDT_BOOLEAN)
{
    m_data.m_boolean = value;
}

Element::Element(ArrayData* value)
: m_parent(NULL), m_type(JDT_ARRAY)
{
    m_data.m_array = value;
    for (Element* jsonElement: *m_data.m_array)
        jsonElement->m_parent = this;
}

Element::Element(ObjectData* value)
: m_parent(NULL), m_type(JDT_OBJECT)
{
    m_data.m_object = value;
    for (auto itor: *m_data.m_object)
        itor.second->m_parent = this;
}

Element::Element(ArrayData& value) {}

Element::Element(ObjectData& value) {}

Element::Element()
: m_parent(NULL), m_type(JDT_NULL)
{
    m_data.m_boolean = false;
}

Element::Element(const Element& other)
{
    assign(other);
}

void Element::moveElement(Element&& other) noexcept
{
    clear();
    m_type = other.m_type;
    m_parent = other.m_parent;
    memcpy(&m_data, &other.m_data, sizeof(m_data));
    other.m_type = JDT_NULL;
}

Element::Element(Element&& other) noexcept
: m_type(JDT_NULL)
{
    moveElement(move(other));
}

void Element::assign(const Element& other)
{
    m_type = other.m_type;
    switch (m_type) {
        case JDT_STRING:
            m_data.m_string = new string(*other.m_data.m_string);
            break;

        case JDT_ARRAY:
        case JDT_OBJECT:
            break;

        default:
            memcpy(&m_data, &other.m_data, sizeof(m_data));
            break;
    }
}

Element& Element::operator = (const Element& other)
{
    assign(other);
    return *this;
}

Element& Element::operator = (Element&& other)
{
    moveElement(move(other));
    return *this;
}

void Element::clear()
{
    switch (m_type) {
        case JDT_STRING:
            delete m_data.m_string;
            break;

        case JDT_ARRAY:
            if (m_data.m_array)
                delete m_data.m_array;
            break;

        case JDT_OBJECT:
            if (m_data.m_object)
                delete m_data.m_object;
            break;

        default:
            break;
    }
    m_type = JDT_NULL;
}

Element::~Element()
{
    clear();
}

void Element::add(Element* element)
{
    if (m_type != JDT_ARRAY)
        throw Exception("Parent element is not JSON array");
    if (!m_data.m_array)
        m_data.m_array = new ArrayData(this);
    m_data.m_array->add(element);
    element->m_parent = this;
}

void Element::add(string name, Element* element)
{
    if (m_type != JDT_OBJECT)
        throw Exception("Parent element is not JSON object");

    if (!m_data.m_object)
        m_data.m_object = new ObjectData(this);

    m_data.m_object->add(name, element);
    element->m_parent = this;
}

const Element* Element::find(const string& name) const
{
    if (m_type != JDT_OBJECT)
        throw Exception("Parent element is nether JSON array nor JSON object");
    if (!m_data.m_object)
        return NULL;
    return m_data.m_object->find(name);
}

Element* Element::find(const string& name)
{
    if (m_type != JDT_OBJECT)
        throw Exception("Parent element is nether JSON array nor JSON object");
    if (!m_data.m_object)
        return NULL;
    return m_data.m_object->find(name);
}

Element& Element::operator[](const std::string& name) throw (Exception)
{
    if (m_type != JDT_OBJECT && m_type != JDT_NULL)
        throw Exception("Parent element is not JSON object");

    if (m_type == JDT_NULL || !m_data.m_object) {
        m_data.m_object = new ObjectData(this);
        m_type = JDT_OBJECT;
    }

    return (*m_data.m_object)[name];
}

const Element& Element::operator[](const std::string& name) const throw (Exception)
{
    if (m_type != JDT_OBJECT)
        return emptyElement;

    const Element* element = find(name);
    if (!element)
        return emptyElement;
    return *element;
}

Element& Element::operator[](size_t index) throw (Exception)
{
    if (m_type != JDT_ARRAY)
        throw Exception("Parent element is not JSON array");

    if (!m_data.m_array)
        m_data.m_array = new ArrayData(this);

    while (index <= m_data.m_array->size())
        m_data.m_array->add(new Element(""));

    return (*m_data.m_array)[index];
}

const Element& Element::operator[](size_t index) const throw (Exception)
{
    if (m_type != JDT_ARRAY)
        throw Exception("Parent element is not JSON array");

    if (!m_data.m_array || index >= m_data.m_array->size())
        throw Exception("JSON array index out of bound");

    return (*m_data.m_array)[index];
}

void Element::remove(const string& name)
{
    if (m_type != JDT_OBJECT)
        throw Exception("Parent element is not JSON object");
    if (!m_data.m_object)
        return;
    m_data.m_object->remove(name);
}

Element* Element::parent()
{
    return m_parent;
}

Type Element::type() const
{
    return m_type;
}

double Element::getNumber() const
{
    if (m_type == JDT_NUMBER)
        return m_data.m_number;
    throw Exception("Not a number");
}

string Element::getString() const
{
    if (m_type == JDT_STRING)
        return *m_data.m_string;
    stringstream output;
    exportValueTo(output, false, 0);
    return output.str();
}

bool Element::getBoolean() const
{
    if (m_type == JDT_STRING)
        return *m_data.m_string == "true";
    else if (m_type == JDT_BOOLEAN)
        return m_data.m_boolean;
    throw Exception("Not a boolean");
}

json::ArrayData& Element::getArray()
{
    if (m_type == JDT_ARRAY && m_data.m_array)
        return *m_data.m_array;
    throw Exception("Not an array");
}

const json::ArrayData& Element::getArray() const
{
    if (m_type == JDT_ARRAY && m_data.m_array)
        return *m_data.m_array;
    throw Exception("Not an array");
}

json::ObjectData& Element::getObject()
{
    if (m_type == JDT_OBJECT && m_data.m_object)
        return *m_data.m_object;
    throw Exception("Not an object");
}

const json::ObjectData& Element::getObject() const
{
    if (m_type == JDT_OBJECT && m_data.m_object)
        return *m_data.m_object;
    throw Exception("Not an object");
}

void Element::exportValueTo(ostream& stream, bool formatted, int indent) const
{
    string indentSpaces, newLineChar, firstElement(" "), betweenElements(", ");
    if (formatted && m_type & (JDT_ARRAY|JDT_OBJECT)) {
        if (indent)
            indentSpaces = string(indent, ' ');
        newLineChar = "\n";
        firstElement = "\n    " + indentSpaces;
        betweenElements = ",\n    " + indentSpaces;
    }
    switch (m_type) {
        case JDT_NUMBER:
            if (m_data.m_number == (long) m_data.m_number)
                stream << fixed << (long) m_data.m_number;
            else
                stream << fixed << m_data.m_number;
            break;
        case JDT_STRING:
            stream << "\"" << escape(*m_data.m_string) << "\"";
            break;
        case JDT_BOOLEAN:
            stream << (m_data.m_boolean ? "true" : "false");
            break;
        case JDT_ARRAY:
            stream << "[";
            if (m_data.m_array) {
                bool first = true;
                for (Element* element: *m_data.m_array) {
                    if (first) {
                        first = false;
                        stream << firstElement;
                    } else
                        stream << betweenElements;
                    element->exportValueTo(stream, formatted, indent + 4);
                }
                stream << " ";
            }
            stream << "]";
            break;
        case JDT_OBJECT:
            stream << "{";
            if (m_data.m_object) {
                bool first = true;
                for (auto& itor: *m_data.m_object) {
                    if (first) {
                        first = false;
                        stream << firstElement;
                    } else
                        stream << betweenElements;
                    stream << "\"" << itor.first << "\": ";
                    itor.second->exportValueTo(stream, formatted, indent + 4);
                }
                stream << " ";
            }
            stream << newLineChar << indentSpaces << "}";
            break;
        case JDT_NULL:
            stream << "null";
            break;
    }
}

void Element::exportTo(ostream& stream, bool formatted)
{
    exportValueTo(stream, formatted, 0);
}

string Element::escape(const string& text)
{
    string result;

    size_t position = 0;

    for (;;) {
        size_t pos = text.find_first_of("\"\\/\b\f\n\r\t", position);
        if (pos == string::npos) {
            if (position == 0)
                return text;
            result += text.substr(position);
            break;
        }
        result += text.substr(position, pos - position);
        switch (text[pos]) {
            case '"':   result += "\\\""; break;
            case '\\':  result += "\\\\"; break;
            case '/':   result += "\\/"; break;
            case '\b':  result += "\\b"; break;
            case '\f':  result += "\\f"; break;
            case '\n':  result += "\\n"; break;
            case '\r':  result += "\\r"; break;
            case '\t':  result += "\\t"; break;
            default:
                throw runtime_error("Unknown escape character");
        }
        position = pos + 1;
    }

    return result;
}

static std::string codePointToUTF8(unsigned cp)
{
   std::string result;

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
      result[1] = 0x80 | static_cast<char>((0x3f & (cp >> 6)));
      result[0] = 0xE0 | static_cast<char>((0xf & (cp >> 12)));
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

string Element::decode(const string& text)
{
    string result;

    size_t length = text.length();
    size_t position = 0;

    while (position < length) {
        size_t pos = text.find_first_of("\\", position);
        if (pos == string::npos) {
            if (position == 0)
                return text;
            result += text.substr(position);
            break;
        }
        if (pos != position)
            result += text.substr(position, pos - position);
        pos++;
        switch (text[pos]) {
            case '"':   result += '"'; break;
            case '\\':  result += '\\'; break;
            case '/':   result += '/'; break;
            case 'b':  result += '\b'; break;
            case 'f':  result += '\f'; break;
            case 'n':  result += '\n'; break;
            case 'r':  result += '\r'; break;
            case 't':  result += '\t'; break;
            case 'u':
            {
                pos++;
                string ucharCodeStr = text.substr(pos, 4);
                unsigned ucharCode = strtol(ucharCodeStr.c_str(), NULL, 16);
                pos += 3;
                result += codePointToUTF8(ucharCode);
                break;
            }
            default:
                throw runtime_error("Unknown escape character");
        }
        position = pos + 1;
    }

    return result;
}

void Element::selectElements(ElementSet& elements, const Strings& xpath, size_t xpathPosition, bool rootOnly)
{
    string xpathElement(xpath[xpathPosition]);
    bool matchAnyElement = xpathElement == "*";
    bool lastPosition = xpath.size() == xpathPosition + 1;

    if (m_type == JDT_ARRAY) {
        for (Element* element: *m_data.m_array) {
            // Continue to match children
            if (!matchAnyElement)
                xpathPosition = 0; // Start over to match children
            element->selectElements(elements, xpath, xpathPosition, false);
        }
    }
    else if (m_type == JDT_OBJECT) {
        if (!matchAnyElement) {
            Element* element = find(xpathElement);
            if (element) {
                if (lastPosition) {
                    // Full xpath match
                    elements.insert(element);
                }
                else {
                    // Continue to match children
                    element->selectElements(elements, xpath, xpathPosition + 1, false);
                }
            }
        } else {
            for (auto& itor: *m_data.m_object) {
                if (lastPosition) {
                    // Full xpath match
                    Element* element = itor.second;
                    elements.insert(element);
                }
                else {
                    // Continue to match children
                    itor.second->selectElements(elements, xpath, xpathPosition + 1, false);
                }
            }
        }

        if (!rootOnly) {
            for (auto& itor: *m_data.m_object) {
                if (itor.second->m_type == JDT_OBJECT)
                    itor.second->selectElements(elements, xpath, 0, false);
            }
        }
    }
}

void Element::select(ElementSet& elements, std::string xpath)
{
    bool rootOnly = false;
    if (xpath[0] == '/') {
        xpath = xpath.substr(1);
        if (xpath[0] == '/')
            xpath = xpath.substr(1);
        else
            rootOnly = true;
    }
    Strings pathElements(xpath, "/");
    elements.clear();
    selectElements(elements, pathElements, 0, rootOnly);
}
