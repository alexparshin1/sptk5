/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonElement.cpp - description                          ║
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

#include <sptk5/json/JsonElement.h>
#include <sptk5/json/JsonArrayData.h>
#include <sstream>
#include <cstring>

using namespace std;
using namespace sptk;
using namespace sptk::json;

const Element Element::emptyElement("");

Element::Element(double value) noexcept
: m_parent(nullptr), m_type(JDT_NUMBER)
{
    m_data.m_number = value;
}

Element::Element(int value) noexcept
: m_parent(nullptr), m_type(JDT_NUMBER)
{
    m_data.m_number = value;
}

Element::Element(int64_t value) noexcept
: m_parent(nullptr), m_type(JDT_NUMBER)
{
    m_data.m_number = (double) value;
}

Element::Element(const std::string& value) noexcept
: m_parent(nullptr), m_type(JDT_STRING)
{
    m_data.m_string = new string(value);
}

Element::Element(const char* value) noexcept
: m_parent(nullptr), m_type(JDT_STRING)
{
    m_data.m_string = new string(value);
}

Element::Element(bool value) noexcept
: m_parent(nullptr), m_type(JDT_BOOLEAN)
{
    m_data.m_boolean = value;
}

Element::Element(ArrayData* value) noexcept
: m_parent(nullptr), m_type(JDT_ARRAY)
{
    m_data.m_array = value;
    for (Element* jsonElement: *m_data.m_array)
        jsonElement->m_parent = this;
}

Element::Element(ObjectData* value) noexcept
: m_parent(nullptr), m_type(JDT_OBJECT)
{
    m_data.m_object = value;
    for (auto itor: *m_data.m_object)
        itor.second->m_parent = this;
}

Element::Element(ArrayData& value)
: m_parent(nullptr), m_type(JDT_NULL)
{}

Element::Element(ObjectData& value)
: m_parent(nullptr), m_type(JDT_NULL)
{}

Element::Element() noexcept
: m_parent(nullptr), m_type(JDT_NULL)
{
    m_data.m_boolean = false;
}

Element::Element(const Element& other)
: m_parent(nullptr), m_type(JDT_NULL)
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

Element& Element::operator=(const Element& other)
{
    assign(other);
    return *this;
}

Element& Element::operator=(Element&& other) noexcept
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
            delete m_data.m_array;
            break;

        case JDT_OBJECT:
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

Element* Element::add(Element* element)
{
    if (m_type != JDT_ARRAY)
        throw Exception("Parent element is not JSON array");
    if (!m_data.m_array)
        m_data.m_array = new ArrayData(this);
    m_data.m_array->add(element);
    element->m_parent = this;

    return element;
}

Element* Element::add(const String& name, Element* element)
{
    if (m_type != JDT_OBJECT)
        throw Exception("Parent element is not JSON object");

    if (!m_data.m_object)
        m_data.m_object = new ObjectData(this);

    Element* sameNameExistingElement = m_data.m_object->find(name);
    if (sameNameExistingElement == nullptr) {
        m_data.m_object->add(name, element);
        element->m_parent = this;
        return element;
    }

    if (sameNameExistingElement->isArray()) {
        sameNameExistingElement->add(element);
        return element;
    }

    m_data.m_object->move(name);
    auto array = new Element(new ArrayData());
    array->add(sameNameExistingElement);
    array->add(element);
    add(name, array);

    return element;
}

const Element* Element::find(const String& name) const
{
    if (m_type != JDT_OBJECT)
        throw Exception("Parent element is nether JSON array nor JSON object");
    if (!m_data.m_object)
        return nullptr;
    return m_data.m_object->find(name);
}

Element* Element::find(const String& name)
{
    if (m_type != JDT_OBJECT)
        throw Exception("Parent element is nether JSON array nor JSON object");
    if (!m_data.m_object)
        return nullptr;
    return m_data.m_object->find(name);
}

Element& Element::operator[](const char* name)
{
    if (m_type != JDT_OBJECT && m_type != JDT_NULL)
        throw Exception("Parent element is not JSON object");

    if (m_type == JDT_NULL || !m_data.m_object) {
        m_data.m_object = new ObjectData(this);
        m_type = JDT_OBJECT;
    }

    return (*m_data.m_object)[name];
}

const Element& Element::operator[](const char* name) const
{
    if (m_type != JDT_OBJECT)
        return emptyElement;

    const Element* element = find(name);
    if (!element)
        return emptyElement;
    return *element;
}

Element& Element::operator[](const String& name)
{
    if (m_type != JDT_OBJECT && m_type != JDT_NULL)
        throw Exception("Parent element is not JSON object");

    if (m_type == JDT_NULL || !m_data.m_object) {
        m_data.m_object = new ObjectData(this);
        m_type = JDT_OBJECT;
    }

    return (*m_data.m_object)[name];
}

const Element& Element::operator[](const String& name) const
{
    if (m_type != JDT_OBJECT)
        return emptyElement;

    const Element* element = find(name);
    if (!element)
        return emptyElement;
    return *element;
}

Element& Element::operator[](size_t index)
{
    if (m_type != JDT_ARRAY)
        throw Exception("Parent element is not JSON array");

    if (!m_data.m_array)
        m_data.m_array = new ArrayData(this);

    while (index <= m_data.m_array->size())
        m_data.m_array->add(new Element(""));

    return (*m_data.m_array)[index];
}

const Element& Element::operator[](size_t index) const
{
    if (m_type != JDT_ARRAY)
        throw Exception("Parent element is not JSON array");

    if (!m_data.m_array || index >= m_data.m_array->size())
        throw Exception("JSON array index out of bound");

    return (*m_data.m_array)[index];
}

void Element::remove(const String& name)
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

Element& Element::getChild(const String& name)
{
    Element* element = this;
    if (!name.empty()) {
        element = find(name);
        if (element == nullptr)
            throw Exception("Not a number");
    }
    return *element;
}

const Element& Element::getChild(const String& name) const
{
    const Element* element = this;
    if (!name.empty()) {
        element = find(name);
        if (element == nullptr)
            throw Exception("Not a number");
    }
    return *element;
}

double Element::getNumber(const String& name) const
{
    auto& element = getChild(name);
    if (element.m_type == JDT_NUMBER)
        return element.m_data.m_number;
    throw Exception("Not a number");
}

String Element::getString(const String& name) const
{
    auto& element = getChild(name);

    if (element.m_type == JDT_STRING)
        return *element.m_data.m_string;

    switch (element.m_type) {
        case JDT_NUMBER: {
            int len;
            char buffer[64];
            if (element.m_data.m_number == (long) element.m_data.m_number)
                len = snprintf(buffer, sizeof(buffer) - 1, "%ld", (long) element.m_data.m_number);
            else
                len = snprintf(buffer, sizeof(buffer) - 1, "%f", element.m_data.m_number);
            return String(buffer, (size_t) len);
        }

        case JDT_STRING:
            return *element.m_data.m_string;

        case JDT_BOOLEAN:
            return element.m_data.m_boolean ? string("true", 4) : string("false", 5);

        case JDT_NULL:
            return string("null", 4);

        default:
            break;
    }

    stringstream output;
    exportValueTo(output, false, 0);
    return output.str();
}

bool Element::getBoolean(const String& name) const
{
    auto& element = getChild(name);
    if (element.m_type == JDT_STRING)
        return *element.m_data.m_string == "true";
    else if (element.m_type == JDT_BOOLEAN)
        return element.m_data.m_boolean;
    throw Exception("Not a boolean");
}

json::ArrayData& Element::getArray(const String& name)
{
    auto& element = getChild(name);
    if (element.m_type == JDT_ARRAY && element.m_data.m_array)
        return *element.m_data.m_array;
    throw Exception("Not an array");
}

const json::ArrayData& Element::getArray(const String& name) const
{
    auto& element = getChild(name);
    if (element.m_type == JDT_ARRAY && element.m_data.m_array)
        return *element.m_data.m_array;
    throw Exception("Not an array");
}

json::ObjectData& Element::getObject(const String& name)
{
    auto& element = getChild(name);
    if (element.m_type == JDT_OBJECT && element.m_data.m_object)
        return *element.m_data.m_object;
    throw Exception("Not an object");
}

const json::ObjectData& Element::getObject(const String& name) const
{
    auto& element = getChild(name);
    if (element.m_type == JDT_OBJECT && element.m_data.m_object)
        return *element.m_data.m_object;
    throw Exception("Not an object");
}

void Element::exportValueTo(ostream& stream, bool formatted, size_t indent) const
{
    string indentSpaces, newLineChar, firstElement(" "), betweenElements(", ");
    if (formatted && m_type & (JDT_ARRAY | JDT_OBJECT)) {
        if (indent)
            indentSpaces = string(indent, ' ');
        newLineChar = "\n";
        firstElement = "\n    " + indentSpaces;
        betweenElements = ",\n    " + indentSpaces;
    }

    auto saveFlags = stream.flags();

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
    stream.flags(saveFlags);
}

void Element::exportValueTo(const String& name, XMLElement& parentNode) const
{
    auto node = new XMLElement(parentNode, name);
    switch (m_type) {
        case JDT_NUMBER:
        case JDT_BOOLEAN:
        case JDT_STRING:
            node->text(getString());
            break;

        case JDT_ARRAY:
            if (m_data.m_array) {
                for (Element* element: *m_data.m_array)
                    element->exportValueTo("item", *node);
            }
            break;

        case JDT_OBJECT:
            if (m_data.m_object) {
                for (auto& itor: *m_data.m_object)
                    itor.second->exportValueTo(itor.first, *node);
            }
            break;

        case JDT_NULL:
            new XMLElement(node, "null");
            break;
    }
}

void Element::exportTo(ostream& stream, bool formatted) const
{
    exportValueTo(stream, formatted, 0);
}

void Element::exportTo(const string& name, XMLElement& parentNode) const
{
    exportValueTo(name, parentNode);
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

static std::string codePointToUTF8(unsigned cp)
{
    std::string result;

    // based on description from http://en.wikipedia.org/wiki/UTF-8

    if (cp <= 0x7f) {
        result.resize(1);
        result[0] = static_cast<char>(cp);
    } else if (cp <= 0x7FF) {
        result.resize(2);

        result[1] = static_cast<char>(0x80 | (0x3f & cp));
        result[0] = static_cast<char>(0xC0 | (0x1f & (cp >> 6)));
    } else if (cp <= 0xFFFF) {
        result.resize(3);
        result[2] = static_cast<char>(0x80 | (0x3f & cp));
        result[1] = 0x80 | static_cast<char>((0x3f & (cp >> 6)));
        result[0] = 0xE0 | static_cast<char>((0xf & (cp >> 12)));
    } else if (cp <= 0x10FFFF) {
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
        size_t pos = text.find_first_of('\\', position);
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
            case 'u': {
                pos++;
                string ucharCodeStr = text.substr(pos, 4);
                unsigned ucharCode = strtol(ucharCodeStr.c_str(), nullptr, 16);
                pos += 3;
                result += codePointToUTF8(ucharCode);
                break;
            }
            default:
                throw Exception("Unknown escape character");
        }
        position = pos + 1;
    }

    return result;
}

void Element::selectElements(ElementSet& elements, const Strings& xpath, size_t xpathPosition, bool rootOnly)
{
    String xpathElement(xpath[xpathPosition]);
    bool matchAnyElement = xpathElement == String("*", 1);
    bool lastPosition = xpath.size() == xpathPosition + 1;

    if (m_type == JDT_ARRAY) {
        for (Element* element: *m_data.m_array) {
            // Continue to match children
            if (!matchAnyElement)
                xpathPosition = 0; // Start over to match children
            element->selectElements(elements, xpath, xpathPosition, false);
        }
    } else if (m_type == JDT_OBJECT) {
        if (!matchAnyElement) {
            Element* element = find(xpathElement);
            if (element) {
                if (lastPosition) {
                    // Full xpath match
                    elements.insert(element);
                } else {
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
                } else {
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
    if (pathElements.empty()) {
        elements.insert(this);
        return;
    }
    selectElements(elements, pathElements, 0, rootOnly);
}

size_t Element::size() const
{
    if (m_type == JDT_OBJECT)
        return m_data.m_object->size();
    if (m_type == JDT_ARRAY)
        return m_data.m_array->size();
    return 0;
}

void Element::optimizeArrays(const std::string& name)
{
    if (isObject()) {
        if (size() == 1) {
            auto itor = m_data.m_object->begin();
            Element* itemElement = itor->second;
            if ((itor->first == name || name.empty()) && itemElement->isArray()) {
                m_data.m_object->move(itor->first);
                *this = ::move(*itemElement);
                optimizeArrays(name);
                return;
            }
        }
        for (auto itor: *m_data.m_object) {
            Element* element = itor.second;
            element->optimizeArrays(name);
        }
        return;
    }

    if (isArray()) {
        for (auto element: *m_data.m_array)
            element->optimizeArrays(name);
        return;
    }
}
