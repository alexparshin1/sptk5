/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonElement.h - description                            ║
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

#ifndef __JSON__ELEMENT_H__
#define __JSON__ELEMENT_H__

#include <sptk5/sptk.h>
#include <sptk5/Strings.h>
#include <sptk5/Exception.h>
#include <sptk5/json/JsonObjectData.h>
#include <set>

namespace sptk { namespace json {

/// @addtogroup JSON
/// @{

/**
 * JSON Element object pointers.
 * Used in select() method.
 */
typedef std::set<Element*>              ElementSet;

/**
 * JSON Element type
 */
enum Type {
    JDT_NUMBER  = 1,
    JDT_STRING  = 2,
    JDT_BOOLEAN = 4,
    JDT_ARRAY   = 8,
    JDT_OBJECT  = 16,
    JDT_NULL    = 32
};

class ArrayData;

/**
 * JSON Element
 *
 * May contain any type of JSON object
 */
class Element
{
    friend class Document;
    friend class Parser;
    friend class ArrayData;
    friend class ObjectData;
protected:
    /**
     * Parent JSON Element
     */
    Element*    m_parent;

    /**
     * JSON Element type
     */
    Type        m_type;

    /**
     * JSON Element data
     */
    union {
        double          m_number;
        std::string*    m_string;
        bool            m_boolean;
        ArrayData*      m_array;
        ObjectData*     m_object;
    } m_data;

    /**
     * Clear JSON element.
     * Releases memory allocated by string, array, or object data,
     * and sets the type to JDT_NULL.
     */
    void clear();

    /**
     * Assign from another element
     * @param other const Element&, Element to assign from
     */
    void assign(const Element& other);

    /**
     * Move element
     * @param other Element&&, Element to move from
     */
    void moveElement(Element&& other) noexcept;

    /**
     * Export JSON element to text format
     * @param stream std::ostream&, Output stream
     * @param formatted bool, If true then output JSON text is indented. Otherwise, it is using minimal formatting (elements separated with single space).
     * @param indent int, Formatting indent
     */
    void exportValueTo(std::ostream& stream, bool formatted, size_t indent) const;

    /**
     * Empty const Json element
     */
    static const Element    emptyElement;

public:
    /**
     * Escape special characters
     * @param text const std::string&, text with special characters
     * @returns escaped text
     */
    static std::string escape(const std::string& text);

    /**
     * Decode escaped text
     * @param text const std::string&, escaped text
     * @returns decoded text
     */
    static std::string decode(const std::string& text);

    /**
     * Find all child elements matching particular xpath element
     * @param elements ElementVector&, Elements matching xpath (output)
     * @param xpath const Strings&, Xpath elements
     * @param xpathPosition size_t, Position in xpath currently being checked
     * @param rootOnly bool, Flag indicating that only root level elements are checked
     */
    void selectElements(ElementSet& elements, const Strings& xpath, size_t xpathPosition, bool rootOnly);

private:

    /**
     * Blocked constructor
     * @param value const ArrayData&, Array of JSON Elements
     */
    Element(ArrayData& value);

    /**
     * Blocked constructor
     * @param value const ObjectData&, Map of JSON Elements
     */
    Element(ObjectData& value);

public:

    /**
     * Constructor
     * @param value double, Floating point value
     */
    Element(double value);

    /**
     * Constructor
     * @param value int, Integer value
     */
    Element(int value);

    /**
     * Constructor
     * @param value const std::string&, String value
     */
    Element(const std::string& value);

    /**
     * Constructor
     * @param value const char*, String value
     */
    Element(const char* value);

    /**
     * Constructor
     * @param value bool, Boolean value
     */
    Element(bool value);

    /**
     * Constructor
     * Element takes ownership of value.
     * Elements in value are set their parent pointer to this element.
     * @param value const ArrayData*, Array of JSON Elements
     */
    Element(ArrayData* value);

    /**
     * Constructor
     * Element takes ownership of value.
     * Elements in value are set their parent pointer to this element.
     * @param value const ObjectData*, Map of names to JSON elements
     */
    Element(ObjectData* value);

    /**
     * Constructor
     * Element will contain null value
     */
    Element();

    /**
     * Copy constructor
     * @param other const Element&, Element to assign from
     */
    Element(const Element& other);

    /**
     * Move constructor
     * @param other Element&&, Element to assign from
     */
    Element(Element&& other) noexcept;

    /**
     * Destructor
     */
    ~Element();

    /**
     * Assignment operator
     * @param other const Element&, Element to assign from
     */
    Element& operator = (const Element& other);

    /**
     * Assignment operator
     * @param other Element&&, Element to assign from
     */
    Element& operator = (Element&& other);

    /**
     * Add JSON element to JSON array element.
     *
     * Element takes ownership of added element.
     * @param element Element*, Element to add
     */
    void add(Element* element);

    /**
     * Add JSON element to JSON object element.
     *
     * Element takes ownership of added element.
     * @param name const std::string&, Name of the element in the object element
     * @param element Element*, Element to add
     */
    void add(const std::string& name, Element* element);

    /**
     * Add integer field to JSON element
     * @param name const std::string&, Field name
     * @param value int, Field value
     */
    void add(const std::string& name, int value) { add(name, new Element(value)); }

    /**
     * Add double field to JSON element
     * @param name const std::string&, Field name
     * @param value double, Field value
     */
    void add(const std::string& name, double value) { add(name, new Element(value)); }

    /**
     * Add string field to JSON element
     * @param name const std::string&, Field name
     * @param value const std::string&, Field value
     */
    void add(const std::string& name, const std::string& value) { add(name, new Element(value)); }

    /**
     * Add string field to JSON element
     * @param name const std::string&, Field name
     * @param value const char*, Field value
     */
    void add(const std::string& name, const char* value) { add(name, new Element(value)); }

    /**
     * Add boolean field to JSON element
     * @param name const std::string&, Field name
     * @param value boolean, Field value
     */
    void add(const std::string& name, bool value) { add(name, new Element(value)); }

    /**
     * Add JSON array field to JSON element
     * @param name const std::string&, Field name
     * @param value ArrayData*, Field value
     */
    void add(const std::string& name, ArrayData* value) { add(name, new Element(value)); }

    /**
     * Add JSON object field to JSON element
     * @param name const std::string&, Field name
     * @param value ObjectData*, Field value
     *
     */
    void add(const std::string& name, ObjectData* value) { add(name, new Element(value)); }

    /**
     * Find JSON element in JSON object element
     * @param name const std::string&, Name of the element in the object element
     * @returns Element for the name, or NULL if not found
     */
    const Element* find(const std::string& name) const;

    /**
     * Find JSON element in JSON object element
     * @param name const std::string&, Name of the element in the object element
     * @returns Element for the name, or NULL if not found
     */
    Element* find(const std::string& name);

    /**
     * Get JSON element in JSON object element by name.
     * If element doesn't exist in JSON object yet, it's created as JSON null element.
     * If this element is not JSON object, an exception is thrown.
     * @param name const std::string&, Name of the element in the object element
     * @returns Element for the name, or NULL if not found
     */
    Element& operator[](const std::string& name);

    /**
     * Get JSON element in JSON object element by name.
     * If element doesn't exist in JSON object yet, then reference to static const JSON null element is returned.
     * If this element is not JSON object, an exception is thrown.
     * @param name const std::string&, Name of the element in the object element
     * @returns Element for the name, or NULL if not found
     */
    const Element& operator[](const std::string& name) const;

    /**
     * Get JSON element in JSON array element by index.
     * If this element is not JSON array, or an element doesn't exist in JSON array yet, an exception is thrown.
     * @param index uint32_t, Index of the element in the array element
     * @returns Element for the name, or NULL if not found
     */
    Element& operator[](size_t index);

    /**
     * Get JSON element in JSON array element by index.
     * If this element is not JSON array, or an element doesn't exist in JSON array yet, an exception is thrown.
     * @param index uint32_t, Index of the element in the array element
     * @returns Element for the name, or NULL if not found
     */
    const Element& operator[](size_t index) const;

    /**
     * Remove JSON element by name from this JSON object element
     * @param name const std::string&, Name of the element in the object element
     */
    void remove(const std::string& name);

    /**
     * Get parent JSON element
     * @returns Parent JSON Element or NULL (for root element)
     */
    Element* parent();

    /**
     * Get JSON element type
     * @returns JSON element type
     */
    Type type() const;

    /**
     * Get value of JSON element
     */
    double getNumber() const;

    /**
     * Get value of JSON element
     */
    std::string getString() const;

    /**
     * Get value of JSON element
     */
    bool getBoolean() const;

    /**
     * Get value of JSON element.
     * If you want to modify elements of the array inside
     * this element, get array element using [name] and then add() or remove() its item(s).
     * Alternatively, create a new ArrayData object and replace existing one.
     */
    ArrayData& getArray();

    /**
     * Get value of JSON element.
     * If you want to modify elements of the array inside
     * this element, get array element using [name] and then add() or remove() its item(s).
     * Alternatively, create a new ArrayData object and replace existing one.
     */
    const ArrayData& getArray() const;

    /**
     * Get value of JSON element
     * If you want to modify elements of the object inside
     * this element, get object element using [name] and then add() or remove() its item(s).
     * Alternatively, create a new ObjectData object and replace existing one.
     */
    ObjectData& getObject();

    /**
     * Get value of JSON element
     * If you want to modify elements of the object inside
     * this element, get object element using [name] and then add() or remove() its item(s).
     * Alternatively, create a new ObjectData object and replace existing one.
     */
    const ObjectData& getObject() const;

    /**
     * Export JSON element (and all children) to stream
     * @param stream std::ostream&, Stream to export JSON
     * @param formatted bool, If true then JSON text is nicely formatted, but takes more space
     */
    void exportTo(std::ostream& stream, bool formatted=true);

    /** @brief Selects elements as defined by XPath
     *
     * The implementation is just started, so only limited XPath standard part is supported.
     * Currently, examples 1 through 1 from http://www.zvon.org/xxl/XPathTutorial/Output/example1.html
     * are working fine with the exceptions:
     * - no functions are supported yet
     * - no attributes supported, because it isn't XML
     * @param elements ElementPaths&, the resulting list of elements
     * @param xpath std::string, the xpath for elements
     */
    void select(ElementSet& elements, std::string xpath);

    /**
     * Element type check
     * @return true if element is a number
     */
    bool isNumber()  const { return m_type == JDT_NUMBER; }

    /**
     * Element type check
     * @return true if element is a string
     */
    bool isString()  const { return m_type == JDT_STRING; }

    /**
     * Element type check
     * @return true if element is a boolean
     */
    bool isBoolean() const { return m_type == JDT_BOOLEAN; }

    /**
     * Element type check
     * @return true if element is an array
     */
    bool isArray()   const { return m_type == JDT_ARRAY; }

    /**
     * Element type check
     * @return true if element is an object
     */
    bool isObject()  const { return m_type == JDT_OBJECT; }

    /**
     * Element type check
     * @return true if element is a null
     */
    bool isNull()    const { return m_type == JDT_NULL; }
};

}}

#endif
