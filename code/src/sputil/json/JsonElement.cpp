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

#include <sptk5/json/JsonElement.h>
#include <sptk5/json/JsonArrayData.h>
#include <sptk5/json/JsonDocument.h>
#include <cmath>

using namespace std;
using namespace sptk;
using namespace sptk::json;

void ElementData::clear() noexcept
{
    switch (type()) {
        case JDT_ARRAY:
            delete m_data.get_array();
            break;

        case JDT_OBJECT:
            delete m_data.get_object();
            break;

        default:
            break;
    }
    m_type = JDT_NULL;
}

size_t ElementData::size() const
{
    if (m_type == JDT_OBJECT)
        return m_data.get_object()->size();
    if (m_type == JDT_ARRAY)
        return m_data.get_array()->size();
    return 0;
}

const Element* ElementData::find(const String& name) const
{
    if (!is(JDT_OBJECT))
        throw Exception("Parent element is nether JSON array nor JSON object");
    if (!data().get_object())
        return nullptr;
    return data().get_object()->find(name);
}

Element* ElementData::find(const String& name)
{
    if (!is(JDT_OBJECT))
        throw Exception("Parent element is nether JSON array nor JSON object");
    if (!data().get_object())
        return nullptr;
    return data().get_object()->find(name);
}

Element& ElementData::getChild(const String& name)
{
    auto* element = dynamic_cast<Element*>(this);
    if (!name.empty()) {
        element = find(name);
        if (element == nullptr)
            throw Exception("Not a number");
    }
    return *element;
}

const Element& ElementData::getChild(const String& name) const
{
    const auto * element = dynamic_cast<const Element*>(this);
    if (!name.empty()) {
        element = find(name);
        if (element == nullptr)
            throw Exception("No such child element");
    }
    return *element;
}

double ElementGetMethods::getNumber(const String& name) const
{
    const auto & element = getChild(name);
    if (element.is(JDT_NUMBER))
        return element.data().get_number();
    else if (element.is(JDT_STRING))
        return string2double(*element.data().get_string());
    throw Exception("Not a number");
}

static String JsonNumberToString(double number)
{
    long len = 0;
    char buffer[64];

    if (number == round(number))
        len = snprintf(buffer, sizeof(buffer) - 1, "%lld", (long long) number);
    else {
        len = snprintf(buffer, sizeof(buffer) - 1, "%1.8f", number);
        const char* ptr = buffer + len - 1;
        while (*ptr == '0')
            --ptr;
        len = long(ptr - buffer + 1);
    }
    return String(buffer, (size_t) len);
}

String ElementGetMethods::getString(const String& name) const
{
    const auto & element = getChild(name);

    if (element.is(JDT_STRING))
        return *element.data().get_string();

    switch (element.type()) {
        case JDT_NUMBER:
            return JsonNumberToString(element.data().get_number());

        case JDT_STRING:
            return *element.data().get_string();

        case JDT_BOOLEAN:
            return element.data().get_boolean() ? string("true", 4) : string("false", 5);

        case JDT_NULL:
            return string("null", 4);

        default:
            break;
    }

    stringstream output;
    exportValueTo(output, false, 0);
    return output.str();
}

bool ElementGetMethods::getBoolean(const String& name) const
{
    const auto & element = getChild(name);
    if (element.type() == JDT_STRING)
        return *element.data().get_string() == "true";
    else if (element.type() == JDT_BOOLEAN)
        return element.data().get_boolean();
    throw Exception("Not a boolean");
}

json::ArrayData& ElementGetMethods::getArray(const String& name)
{
    auto& element = getChild(name);
    if (element.type() == JDT_ARRAY && element.data().get_array())
        return *element.data().get_array();
    throw Exception("Not an array");
}

const json::ArrayData& ElementGetMethods::getArray(const String& name) const
{
    const auto & element = getChild(name);
    if (element.type() == JDT_ARRAY && element.data().get_array())
        return *element.data().get_array();
    throw Exception("Not an array");
}

json::ObjectData& ElementGetMethods::getObject(const String& name)
{
    auto& element = getChild(name);
    if (element.type() == JDT_OBJECT && element.data().get_object())
        return *element.data().get_object();
    throw Exception("Not an object");
}

const json::ObjectData& ElementGetMethods::getObject(const String& name) const
{
    const auto & element = getChild(name);
    if (element.type() == JDT_OBJECT && element.data().get_object())
        return *element.data().get_object();
    throw Exception("Not an object");
}


void ElementGetMethods::exportValueTo(ostream& stream, bool formatted, size_t indent) const
{
    String indentSpaces;
    String newLineChar;
    String firstElement;
    String betweenElements(",");

    if (formatted && type() & (JDT_ARRAY | JDT_OBJECT)) {
        if (indent)
            indentSpaces = string(indent, ' ');
        newLineChar = "\n";
        firstElement = "\n  " + indentSpaces;
        betweenElements = ",\n  " + indentSpaces;
    }

    auto saveFlags = stream.flags();

    switch (type()) {
        case JDT_NUMBER:
            if (data().get_number() == round(data().get_number()))
                stream << fixed << (long) data().get_number();
            else
                stream << fixed << data().get_number();
            break;

        case JDT_STRING:
            stream << "\"" << escape(*data().get_string()) << "\"";
            break;

        case JDT_BOOLEAN:
            stream << (data().get_boolean() ? "true" : "false");
            break;

        case JDT_ARRAY:
            exportArray(stream, formatted, indent, firstElement, betweenElements, newLineChar, indentSpaces);
            break;

        case JDT_OBJECT:
            exportObject(stream, formatted, indent, firstElement, betweenElements, newLineChar, indentSpaces);
            break;

        default:
            stream << "null";
            break;
    }
    stream.flags(saveFlags);
}

void ElementGetMethods::exportValueTo(const String& name, xml::Element& parentNode) const
{
    xml::Element* node {nullptr};
    if (type() != JDT_ARRAY)
        node = parentNode.add<xml::Element>(name);
    switch (type()) {
        case JDT_NUMBER:
        case JDT_BOOLEAN:
        case JDT_STRING:
            node->text(getString());
            break;

        case JDT_ARRAY:
            if (data().get_array()) {
                for (const auto* element: *data().get_array())
                    element->exportValueTo(name, parentNode);
            }
            break;

        case JDT_OBJECT:
            if (data().get_object()) {
                for (auto& itor: *data().get_object())
                    itor.element()->exportValueTo(itor.name(), *node);
            }
            break;

        default:
            new xml::Element(node, "null");
            break;
    }
}

void ElementGetMethods::exportArray(ostream& stream, bool formatted, size_t indent, const String& firstElement, const String& betweenElements, const String& newLineChar, const String& indentSpaces) const
{
    stream << "[";
    if (is(JDT_ARRAY) && data().get_array()) {
        bool first = true;
        const auto& array = *data().get_array();
        if (array.empty()) {
            stream << "]";
            return;
        }
        for (const auto* element: array) {
            if (first) {
                first = false;
                stream << firstElement;
            } else
                stream << betweenElements;
            element->exportValueTo(stream, formatted, indent + 2);
        }
    }
    stream << newLineChar << indentSpaces << "]";
}

void ElementGetMethods::exportObject(ostream& stream, bool formatted, size_t indent, const String& firstElement, const String& betweenElements, const String& newLineChar, const String& indentSpaces) const
{
    stream << "{";
    if (is(JDT_OBJECT) && data().get_object()) {
        bool first = true;
        for (auto& itor: *data().get_object()) {
            if (first) {
                first = false;
                stream << firstElement;
            } else
                stream << betweenElements;
            stream << "\"" << itor.name() << "\":";
            if (formatted)
                stream << " ";
            itor.element()->exportValueTo(stream, formatted, indent + 2);
        }
    }
    stream << newLineChar << indentSpaces << "}";
}

void ElementGetMethods::exportTo(ostream& stream, bool formatted) const
{
    exportValueTo(stream, formatted, 0);
}

void ElementGetMethods::exportTo(const string& name, xml::Element& parentNode) const
{
    exportValueTo(name, parentNode);
}


void ElementGetMethods::optimizeArrays(const std::string& name)
{
    if (is(JDT_OBJECT)) {
        if (size() == 1) {
            auto itor = data().get_object()->begin();
            Element* itemElement = itor->element();
            if ((itor->name() == name || name.empty()) && itemElement->is(JDT_ARRAY)) {
                data().get_object()->move(itor->name());
                *this = ::move(*itemElement);
                optimizeArrays(name);
                return;
            }
        }
        for (auto itor: *data().get_object()) {
            Element* element = itor.element();
            element->optimizeArrays(name);
        }
        return;
    }

    if (is(JDT_ARRAY)) {
        for (auto* element: *data().get_array())
            element->optimizeArrays(name);
        return;
    }
}

void ElementData::selectChildElements(ElementSet& elements, const Element::XPath& xpath, bool rootOnly) const
{
    if (!rootOnly) {
        for (auto& itor: *data().get_object()) {
            if (itor.element()->is(JDT_OBJECT|JDT_ARRAY))
                itor.element()->selectElements(elements, xpath, 0, false);
        }
    }
}

void ElementData::selectElements(ElementSet& elements, const XPath& xpath, size_t xpathPosition, bool rootOnly)
{
    const XPathElement& xpathElement(xpath[xpathPosition]);
    bool matchAnyElement = xpathElement.name == String("*", 1);
    bool lastPosition = xpath.size() == xpathPosition + 1;

    if (is(JDT_ARRAY))
        selectArrayElements(elements, xpath, xpathPosition);
    else if (is(JDT_OBJECT))
        selectObjectElements(elements, xpath, xpathPosition, rootOnly, xpathElement, matchAnyElement, lastPosition);
}

void ElementData::selectArrayElements(ElementSet& elements, const ElementData::XPath& xpath, size_t xpathPosition)
{
    for (Element* element: *data().get_array()) {
        // Continue to match children
        element->selectElements(elements, xpath, xpathPosition, false);
    }
}

void ElementData::selectObjectElements(ElementSet& elements, const ElementData::XPath& xpath, size_t xpathPosition,
                                       bool rootOnly, const ElementData::XPathElement& xpathElement,
                                       bool matchAnyElement, bool lastPosition)
{
    if (!matchAnyElement) {
        Element* element = find(xpathElement.name);
        if (element) {
            if (lastPosition) {
                // Full xpath match
                appendMatchedElement(elements, xpathElement, element);
            } else {
                // Continue to match children
                element->selectElements(elements, xpath, xpathPosition + 1, false);
            }
        }
    } else {
        for (auto& itor: *data().get_object()) {
            if (lastPosition) {
                // Full xpath match
                auto* element = itor.element();
                appendMatchedElement(elements, xpathElement, element);
            } else {
                // Continue to match children
                itor.element()->selectElements(elements, xpath, xpathPosition + 1, false);
            }
        }
    }

    selectChildElements(elements, xpath, rootOnly);
}

void ElementData::appendMatchedElement(ElementSet& elements, const ElementData::XPathElement& xpathElement, json::Element* element)
{
    if (element->type() != JDT_ARRAY) {
        elements.push_back(element);
        return;
    }

    ArrayData& arrayData = element->getArray();
    if (!arrayData.empty()) {
        switch (xpathElement.index) {
            case 0:
                for (auto* item: arrayData)
                    elements.push_back(item);
                break;
            case -1:
                elements.push_back(&arrayData[arrayData.size() - 1]);
                break;
            default:
                elements.push_back(&arrayData[size_t(xpathElement.index) - 1]);
                break;
        }
    }
}

void ElementData::remove(const String& name)
{
    if (type() != JDT_OBJECT)
        throw Exception("Parent element is not JSON object");
    if (!data().get_object())
        return;
    data().get_object()->remove(name);
}

void ElementData::assign(const Element& other)
{
    setType(other.type());
    switch (type()) {
        case JDT_STRING:
            m_data.set_string(new string(*other.data().get_string()));
            break;

        case JDT_ARRAY:
        case JDT_OBJECT:
            break;

        default:
            memcpy(&m_data, &other.m_data, sizeof(m_data));
            break;
    }
}

void ElementData::moveElement(Element&& other) noexcept
{
    m_data = move(other.m_data);
    m_type = exchange(other.m_type, JDT_NULL);
    m_parent = exchange(other.m_parent, nullptr);
}

void ElementData::select(ElementSet& elements, const String& xPath)
{
    XPath xpath(xPath);
    elements.clear();
    if (xpath.empty()) {
        elements.push_back((Element*)this);
        return;
    }

    selectElements(elements, xpath, 0, xpath.rootOnly);
}

Element::Element(Document* document, double value) noexcept
: ElementGetMethods(document, JDT_NUMBER)
{
    data().set_number(value);
}

Element::Element(Document* document, int value) noexcept
: ElementGetMethods(document, JDT_NUMBER)
{
    data().set_number(value);
}

Element::Element(Document* document, int64_t value) noexcept
: ElementGetMethods(document, JDT_NUMBER)
{
    data().set_number((double) value);
}

Element::Element(Document* document, const String& value) noexcept
: ElementGetMethods(document, JDT_STRING)
{
    data().set_string(ElementData::getDocument()->getString(value));
}

Element::Element(Document* document, const char* value) noexcept
: ElementGetMethods(document, JDT_STRING)
{
    data().set_string(ElementData::getDocument()->getString(value));
}

Element::Element(Document* document, bool value) noexcept
: ElementGetMethods(document, JDT_BOOLEAN)
{
    data().set_boolean(value);
}

Element::Element(Document* document, ArrayData* value) noexcept
: ElementGetMethods(document, JDT_ARRAY)
{
    data().set_array(value);
    for (Element* jsonElement: *data().get_array())
        jsonElement->setParent(this);
}

Element::Element(Document* document, ObjectData* value) noexcept
: ElementGetMethods(document, JDT_OBJECT)
{
    data().set_object(value);
    for (auto itor: *data().get_object())
        itor.element()->setParent(this);
}

Element::Element(Document* document) noexcept
: ElementGetMethods(document, JDT_NULL)
{
    data().set_boolean(false);
}

Element::Element(Document* document, const Element& other)
: ElementGetMethods(document, JDT_NULL)
{
    assign(other);
}

Element::Element(Document*document, Element&& other) noexcept
: ElementGetMethods(document, JDT_NULL)
{
	data().set_boolean(false);
    moveElement(move(other));
}

Element& Element::operator=(const Element& other)
{
    if (&other != this)
        assign(other);
    return *this;
}

Element& Element::operator=(Element&& other) noexcept
{
    moveElement(move(other));
    return *this;
}

Element::~Element() noexcept
{
    clear();
}

Element* Element::add(Element* element)
{
    element->setDocument(getDocument());

    if (type() != JDT_ARRAY)
        throw Exception("Parent element is not JSON array");
    if (!data().get_array())
        data().set_array(new ArrayData(getDocument(), this));
    data().get_array()->add(element);
    element->setParent(this);

    return element;
}

Element* Element::add(const String& name, Element* element)
{
    element->setDocument(getDocument());

    if ((type() & (JDT_OBJECT|JDT_NULL)) == 0)
        throw Exception("Parent element is not JSON object");

    if (!data().get_object() || is(JDT_NULL)) {
        setType(JDT_OBJECT);
        data().set_object(new ObjectData(getDocument(), this));
    }

    Element* sameNameExistingElement = data().get_object()->find(name);
    if (sameNameExistingElement == nullptr) {
        data().get_object()->add(name, element);
        element->setParent(this);
        return element;
    }

    if (sameNameExistingElement->is(JDT_ARRAY)) {
        sameNameExistingElement->add(element);
        return element;
    }

    data().get_object()->move(name);
    auto* arrayData = new ArrayData(getDocument());
    auto* array = new Element(getDocument(), arrayData);
    array->add(sameNameExistingElement);
    array->add(element);
    add(name, array);

    return element;
}

Element& Element::operator[](const char* name)
{
    if (!is(JDT_OBJECT|JDT_NULL))
        throw Exception("Parent element is not JSON object");

    if (is(JDT_NULL) || !data().get_object()) {
        data().set_object(new ObjectData(getDocument(), this));
        setType(JDT_OBJECT);
    }

    return (*data().get_object())[name];
}

const Element& Element::operator[](const char* name) const
{
    if (type() != JDT_OBJECT)
        return getDocument()->getEmptyElement();

    const Element* element = find(name);
    if (!element)
        return getDocument()->getEmptyElement();
    return *element;
}

Element& Element::operator[](const String& name)
{
    if (type() != JDT_OBJECT && type() != JDT_NULL)
        throw Exception("Parent element is not JSON object");

    if (type() == JDT_NULL || !data().get_object()) {
        data().set_object(new ObjectData(getDocument(), this));
        setType(JDT_OBJECT);
    }

    return (*data().get_object())[name];
}

const Element& Element::operator[](const String& name) const
{
    if (type() != JDT_OBJECT)
        return getDocument()->getEmptyElement();

    const Element* element = find(name);
    if (!element)
        return getDocument()->getEmptyElement();
    return *element;
}

Element& Element::operator[](size_t index)
{
    if (type() != JDT_ARRAY)
        throw Exception("Element is not JSON array");

    if (!data().get_array())
        data().set_array(new ArrayData(getDocument(), this));

    while (index >= data().get_array()->size())
        data().get_array()->add(new Element(getDocument(), ""));

    return (*data().get_array())[index];
}

const Element& Element::operator[](size_t index) const
{
    if (type() != JDT_ARRAY)
        throw Exception("Element is not JSON array");

    if (!data().get_array() || index >= data().get_array()->size())
        throw Exception("JSON array index out of bound");

    return (*data().get_array())[index];
}

string json::escape(const string& text)
{
    string result;

    size_t position = 0;

    for (;;) {
        size_t pos = text.find_first_of("\"\\\b\f\n\r\t", position);
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
        result[1] = char( 0x80 | static_cast<char>((0x3f & (cp >> 6))) );
        result[0] = char( 0xE0 | static_cast<char>((0xf & (cp >> 12))) );
    } else if (cp <= 0x10FFFF) {
        result.resize(4);
        result[3] = static_cast<char>(0x80 | (0x3f & cp));
        result[2] = static_cast<char>(0x80 | (0x3f & (cp >> 6)));
        result[1] = static_cast<char>(0x80 | (0x3f & (cp >> 12)));
        result[0] = static_cast<char>(0xF0 | (0x7 & (cp >> 18)));
    }

    return result;
}

string json::decode(const string& text)
{
    string   result;
    size_t   length = text.length();
    size_t   position = 0;
    unsigned ucharCode;

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
        ++pos;
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

Element::XPath::XPath(const sptk::String& _xpath)
{
    String xpath(_xpath);
    if (xpath[0] == '/') {
        xpath = xpath.substr(1);
        if (xpath[0] == '/')
            xpath = xpath.substr(1);
        else
            rootOnly = true;
    }
    Strings pathElements(xpath, "/");
    RegularExpression parsePathElement(R"(([^\[]+)(\[(\d+|last\(\))\])?)");
    for (const auto& pathElement: pathElements) {
        auto matches = parsePathElement.m(pathElement);
        if (!matches)
            throw Exception("Unsupported XPath element");
        int index = 0;
        if (matches.groups().size() > 2)
            index = matches[2].value == "last()" ? -1 : string2int(matches[2].value);
        emplace_back(matches[0].value, index);
    }
}

Element* Element::push_array()
{
    auto* arrayElement = new Element(getDocument(), new ArrayData(getDocument()));
    add(arrayElement);
    return arrayElement;
}

Element* Element::add_array(const String& name)
{
    auto* arrayElement = new Element(getDocument(), new ArrayData(getDocument()));
    add(name, arrayElement);
    return arrayElement;
}

Element* Element::push_object()
{
    auto* objectElement = new Element(getDocument(), new ObjectData(getDocument()));
    add(objectElement);
    return objectElement;
}

Element* Element::add_object(const String& name)
{
    auto* objectElement = new Element(getDocument(), new ObjectData(getDocument()));
    add(name, objectElement);
    return objectElement;
}

#if USE_GTEST
static const String testJSON1(R"({ "AAA": { "BBB": "", "CCC": "", "BBB": "", "BBB": "", "DDD": { "BBB": "" }, "CCC": "" } })");
static const String testJSON2(R"({ "AAA": { "BBB": "", "CCC": "", "BBB": "", "DDD": { "BBB": "" }, "CCC": { "DDD": { "BBB": "", "BBB": "" } } } })");
static const String testJSON3(R"({ "AAA": { "XXX": { "DDD": { "BBB": "", "BBB": "", "EEE": null, "FFF": null } }, "CCC": { "DDD": { "BBB": null, "BBB": null, "EEE": null, "FFF": null } }, "CCC": { "BBB": { "BBB": { "BBB": null } } } } })");

static const String testJSON4(R"({ "AAA": { "BBB": "1", "BBB": "2", "BBB": "3", "BBB": "4" } })");
static const String testJSON5(R"({"data":{"type":1,"array":[1,2,3,"test"]}})");

static const String testJSON6(R"({
  "data": {
    "type": 1,
    "array": [
      1,
      2,
      3,
      "test"
    ]
  }
})");

TEST(SPTK_JsonElement, select)
{
    json::ElementSet elementSet;
    json::Document   document;

    document.load(testJSON1);

    document.root().select(elementSet, "/AAA");
    EXPECT_EQ(size_t(1), elementSet.size());

    document.root().select(elementSet, "/AAA/CCC");
    EXPECT_EQ(size_t(2), elementSet.size());

    document.root().select(elementSet, "/AAA/DDD/BBB");
    EXPECT_EQ(size_t(1), elementSet.size());
}

TEST(SPTK_JsonElement, select2)
{
    json::ElementSet elementSet;
    json::Document   document;

    document.load(testJSON2);

    document.root().select(elementSet, "//BBB");
    EXPECT_EQ(size_t(5), elementSet.size());

    document.root().select(elementSet, "//DDD/BBB");
    EXPECT_EQ(size_t(3), elementSet.size());
}

TEST(SPTK_JsonElement, select3)
{
    json::ElementSet elementSet;
    json::Document   document;

    document.load(testJSON3);

    document.root().select(elementSet, "/AAA/CCC/DDD/*");
    EXPECT_EQ(size_t(4), elementSet.size());

    document.root().select(elementSet, "//*");
    EXPECT_EQ(size_t(17), elementSet.size());
}

TEST(SPTK_JsonElement, select4)
{
    json::ElementSet elementSet;
    json::Document   document;

    document.load(testJSON4);

    document.root().select(elementSet, "/AAA/BBB[1]");
    EXPECT_EQ(size_t(1), elementSet.size());
    EXPECT_STREQ("1", elementSet[0]->getString().c_str());

    document.root().select(elementSet, "/AAA/BBB[last()]");
    EXPECT_EQ(size_t(1), elementSet.size());
    EXPECT_STREQ("4", elementSet[0]->getString().c_str());
}

TEST(SPTK_JsonElement, export)
{
    json::Document   document;

    document.load(testJSON5);

    Buffer buffer;
    document.exportTo(buffer, false);
    EXPECT_STREQ(testJSON5.c_str(), buffer.c_str());
    document.exportTo(buffer, true);
    EXPECT_STREQ(testJSON6.c_str(), buffer.c_str());
}

TEST(SPTK_JsonElement, array)
{
    json::Document   document1;
    document1.load(R"([1,2,3,4])");

    json::Document   document2;
    document2.load(R"({"items":[1,2,3,4]})");

    vector<json::Document> documents;
    documents.push_back(move(document1));
    documents.push_back(move(document2));

    int i = 0;
    for (auto& document: documents) {
        String name;
        if (i == 1)
            name = "items";
        auto& array = document.root().getArray(name);
        EXPECT_EQ(size_t(4), array.size());
        EXPECT_EQ(2, (int) array[1].getNumber());

        array.remove(1);
        EXPECT_EQ(size_t(3), array.size());
        EXPECT_EQ(3, (int) array[1].getNumber());

        try {
            auto val = array[3].getNumber();
            FAIL() << "Got value " << val << ", but expecting out of bound";
        }
        catch (const Exception&) {
            SUCCEED() << "Ok: index out of bound";
        }

        const auto& constArray = document.root().getArray(name);
        EXPECT_EQ(3, (int) constArray[1].getNumber());

        json::Element* embeddedArrayElement = nullptr;
        if (name.empty())
            embeddedArrayElement = document.root().push_array();
        else
            embeddedArrayElement = document.root()[name].push_array();
        auto& embeddedArrayData = embeddedArrayElement->getArray();
        embeddedArrayData.add(new json::Element(&document, 123));
        embeddedArrayData.add(new json::Element(&document, "Test"));
        EXPECT_EQ(size_t(4), array.size());
        EXPECT_EQ(size_t(2), embeddedArrayData.size());

        EXPECT_DOUBLE_EQ(123.0, (*embeddedArrayElement)[size_t(0)].getNumber());
        EXPECT_STREQ("Test", (*embeddedArrayElement)[1].getString().c_str());

        ++i;
    }
}

TEST(SPTK_JsonElement, encodeDecode)
{
    String testString("\n\r\t\b\f\"\\/");
    String escapedString = escape(testString);
    EXPECT_STREQ(escapedString.c_str(), R"(\n\r\t\b\f\"\\/)");
    String decodedString = decode(escapedString);
    EXPECT_EQ(decodedString, testString);

    escapedString = R"(\u004015\u00b0C 3\u1C87)";
    decodedString = decode(escapedString);
    EXPECT_STREQ(decodedString.c_str(), "@15°C 3ᲇ");
}

#endif
