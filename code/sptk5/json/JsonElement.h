/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonElement.h - description                            ║
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

#ifndef __JSON__ELEMENT_H__
#define __JSON__ELEMENT_H__

#include <sptk5/cxml>
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
typedef std::vector<Element*> ElementSet;

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
     * Parent JSON document
     */
    Document*       m_document;

    /**
     * Parent JSON element
     */
    Element*        m_parent;

    /**
     * JSON element type
     */
    Type            m_type;

    /**
     * JSON element data
     */
    union {
        double              m_number;
        const std::string*  m_string;
        bool                m_boolean;
        ArrayData*          m_array;
        ObjectData*         m_object;
    } m_data;

    /**
     * XPath element
     */
    struct XPathElement {
        String          name;       ///< Path element name
        int             index {0};  ///< Path element index(position) from start: 1..N - element index, -1 - last element, 0 - don't use
        /**
         * Constructor
         * @param name          Path element name
         * @param index         Path element index(position) from start: 1..N - element index, -1 - last element, 0 - don't use
         */
        XPathElement(const String& name, int index) : name(name), index(index) {}

        /**
         * Copy constructor
         * @param other         Other object to copy from
         */
        XPathElement(const XPathElement& other) = default;
    };

    /**
     * XPath
     */
    struct XPath : public std::vector<XPathElement>
    {
        bool rootOnly {false};          ///< XPath starts from root flag
        /**
         * Constructor
         * @param xpath           XPath expression
         */
        explicit XPath(const String& xpath);
    };

    /**
     * Append matched element to element set
     * @param elements          Matched element set
     * @param xpathElement      Current XPath element
     * @param element           Current element
     */
    void appendMatchedElement(ElementSet& elements, const Element::XPathElement& xpathElement, Element* element);

    /**
     * Clear JSON element.
     * Releases memory allocated by string, array, or object data,
     * and sets the type to JDT_NULL.
     */
    void clear();

    /**
     * Assign from another element
     * @param other             Element to assign from
     */
    void assign(const Element& other);

    /**
     * Move element
     * @param other Element&&, Element to move from
     */
    void moveElement(Element&& other) noexcept;

    /**
     * Export JSON element to text format
     * @param stream            Output stream
     * @param formatted         If true then output JSON text is indented. Otherwise, it is using minimal formatting (elements separated with single space).
     * @param indent            Formatting indent
     */
    void exportValueTo(std::ostream& stream, bool formatted, size_t indent) const;

    /**
     * Export JSON element to XML element
     * @param name              JSON element name
     * @param element           XML element to export to
     */
    void exportValueTo(const String &name, xml::Element &element) const;

public:
    /**
     * Escape special characters
     * @param text              Text with special characters
     * @returns escaped text
     */
    static std::string escape(const std::string& text);

    /**
     * Decode escaped text
     * @param text              Escaped text
     * @returns decoded text
     */
    static std::string decode(const std::string& text);

    /**
     * Find all child elements matching particular xpath element
     * @param elements          Elements matching xpath (output)
     * @param xpath             Xpath elements
     * @param xpathPosition     Position in xpath currently being checked
     * @param rootOnly          Flag indicating that only root level elements are checked
     */
    void selectElements(ElementSet& elements, const XPath& xpath, size_t xpathPosition, bool rootOnly);

private:

    /**
     * Blocked constructor
     * @param document          Parent JSON document
     * @param value             Array of JSON elements
     */
    Element(Document* document, ArrayData& value) = delete;

    /**
     * Blocked constructor
     * @param document          Parent JSON document
     * @param value             Map of JSON Elements
     */
    Element(Document* document, ObjectData& value) = delete;

    /**
     * Get immediate child element, or return this element if the name is empty.
     * Throws an exception if child is not found.
     * @param name              Name of the element in the object element
     */
    Element& getChild(const String& name);

    /**
     * Get immediate child element, or return this element if the name is empty.
     * Throws an exception if child is not found.
     * @param name              Name of the element in the object element
     */
    const Element& getChild(const String& name) const;

public:

    /**
     * Constructor
     * @param document          Parent document
     * @param value             Floating point value
     */
    Element(Document* document, double value) noexcept;

    /**
     * Constructor
     * @param document          Parent document
     * @param value             Integer value
     */
    Element(Document* document, int value) noexcept;

    /**
     * Constructor
     * @param document          Parent document
     * @param value             Integer value
     */
    Element(Document* document, int64_t value) noexcept;

    /**
     * Constructor
     * @param document          Parent document
     * @param value             String value
     */
    Element(Document* document,const String& value) noexcept;

    /**
     * Constructor
     * @param document          Parent document
     * @param value             String value
     */
    Element(Document* document,const char* value) noexcept;

    /**
     * Constructor
     * @param document          Parent document
     * @param value             Boolean value
     */
    Element(Document* document, bool value) noexcept;

    /**
     * Constructor
     * Element takes ownership of value.
     * Elements in value are set their parent pointer to this element.
     * @param document          Parent JSON document
     * @param value             Array of JSON Elements
     */
    Element(Document* document, ArrayData* value) noexcept;

    /**
     * Constructor
     * Element takes ownership of value.
     * Elements in value are set their parent pointer to this element.
     * @param document          Parent JSON document
     * @param value             Map of names to JSON elements
     */
    Element(Document* document, ObjectData* value) noexcept;

    /**
     * Constructor
     * Element will contain null value
     * @param document          Parent JSON document
     */
    Element(Document* document) noexcept;

    /**
     * Constructor
     * @param document          Parent JSON document
     * @param other             Element to assign from
     */
    Element(Document* document, const Element& other);

    /**
     * Move constructor
     * @param document          Parent JSON document
     * @param other             Element to assign from
     */
    Element(Document* document, Element&& other) noexcept;

    /**
     * Destructor
     */
    ~Element();

    /**
     * Assignment operator (template)
     * @tparam T                Data type to assign
     * @param other             Data to assign
     * @return Reference to self
     */
    template <typename T> Element& operator = (const T& other)
    {
        Element element(m_document, other);
        *this = std::move(element);
        return *this;
    }

    /**
     * Assignment operator
     * @param other             Element to assign from
     */
    Element& operator = (const Element& other);

    /**
     * Assignment operator
     * @param other             Element to assign from
     */
    Element& operator = (Element&& other) noexcept;

    /**
     * Add array element to JSON array
     * @return Array element
     */
    Element* push_array();

    /**
     * Add array element to JSON object
     * @param name              Array element name
     * @return Array element
     */
    Element* set_array(const String& name);

    /**
     * Add object element to JSON array
     * @return Object element
     */
    Element* push_object();

    /**
     * Add object element to JSON object
     * @param name              Object element name
     * @return Object element
     */
    Element* set_object(const String& name);

    /**
     * Set null element in JSON object
     * @param name              Element name
     */
    void set(const String& name)
    {
        add(name, new Element(m_document));
    }

    /**
     * Set element in JSON object
     * @param name              Element name
     * @param value             Element value
     */
    template <typename T> void set(const String& name, T value)
    {
        add(name, new Element(m_document, value));
    }

    /**
     * Push null element to JSON array
     */
    void push_back()
    {
        add(new Element(m_document));
    }

    /**
     * Push element to JSON object
     * @param value             Element value
     */
    template <typename T> void push_back(T value)
    {
        add(new Element(m_document, value));
    }

protected:
    /**
     * Add JSON element to JSON array element.
     *
     * Element takes ownership of added element.
     * @param element           Element to add
     */
    Element* add(Element* element);

    /**
     * Add JSON element to JSON object element.
     *
     * Element takes ownership of added element.
     * @param name              Name of the element in the object element
     * @param element           Element to add
     */
    Element* add(const String& name, Element* element);

    /**
     * Add integer field to JSON element
     * @param name              Field name
     * @param value             Field value
     */
    Element* add(const String& name, int value)
    {
        return add(name, new Element(m_document, value));
    }

    /**
     * Add integer field to JSON element
     * @param name              Field name
     * @param value             Field value
     */
    Element* add(const String& name, int64_t value)
    {
        return add(name, new Element(m_document, value));
    }

    /**
     * Add double field to JSON element
     * @param name              Field name
     * @param value             Field value
     */
    Element* add(const String& name, double value)
    {
        return add(name, new Element(m_document, value));
    }

    /**
     * Add string field to JSON element
     * @param name              Field name
     * @param value             Field value
     */
    Element* add(const String& name, const std::string& value)
    {
        return add(name, new Element(m_document, value));
    }

    /**
     * Add string field to JSON element
     * @param name              Field name
     * @param value             Field value
     */
    Element* add(const String& name, const char* value)
    {
        return add(name, new Element(m_document, value));
    }

    /**
     * Add boolean field to JSON element
     * @param name              Field name
     * @param value             Field value
     */
    Element* add(const String& name, bool value)
    {
        return add(name, new Element(m_document, value));
    }

    /**
     * Add JSON array field to JSON element
     * @param name              Field name
     * @param value             Field value
     */
    Element* add(const String& name, ArrayData* value)
    {
        return add(name, new Element(m_document, value));
    }

    /**
     * Add JSON object field to JSON element
     * @param name              Field name
     * @param value             Field value
     *
     */
    Element* add(const String& name, ObjectData* value)
    {
        return add(name, new Element(m_document, value));
    }

public:
    /**
     * Find JSON element in JSON object element
     * @param name              Name of the element in the object element
     * @returns Element for the name, or NULL if not found
     */
    const Element* find(const String& name) const;

    /**
     * Find JSON element in JSON object element
     * @param name              Name of the element in the object element
     * @returns Element for the name, or NULL if not found
     */
    Element* find(const String& name);

    /**
     * Get JSON element in JSON object element by name.
     * If element doesn't exist in JSON object yet, it's created as JSON null element.
     * If this element is not JSON object, an exception is thrown.
     * @param name              Name of the element in the object element
     * @returns Element for the name, or NULL if not found
     */
    Element& operator[](const char* name);

    /**
     * Get JSON element in JSON object element by name.
     * If element doesn't exist in JSON object yet, then reference to static const JSON null element is returned.
     * If this element is not JSON object, an exception is thrown.
     * @param name              Name of the element in the object element
     * @returns Element for the name, or NULL if not found
     */
    const Element& operator[](const char* name) const;

    /**
     * Get JSON element in JSON object element by name.
     * If element doesn't exist in JSON object yet, it's created as JSON null element.
     * If this element is not JSON object, an exception is thrown.
     * @param name              Name of the element in the object element
     * @returns Element for the name, or NULL if not found
     */
    Element& operator[](const String &name);

    /**
     * Get JSON element in JSON object element by name.
     * If element doesn't exist in JSON object yet, then reference to static const JSON null element is returned.
     * If this element is not JSON object, an exception is thrown.
     * @param name              Name of the element in the object element
     * @returns Element for the name, or NULL if not found
     */
    const Element& operator[](const String& name) const;

    /**
     * Get JSON element in JSON array element by index.
     * If this element is not JSON array, or an element doesn't exist in JSON array yet, an exception is thrown.
     * @param index             Index of the element in the array element
     * @returns Element for the name, or NULL if not found
     */
    Element& operator[](size_t index);

    /**
     * Get JSON element in JSON array element by index.
     * If this element is not JSON array, or an element doesn't exist in JSON array yet, an exception is thrown.
     * @param index             Index of the element in the array element
     * @returns Element for the name, or NULL if not found
     */
    const Element& operator[](size_t index) const;

    /**
     * Remove JSON element by name from this JSON object element
     * @param name              Name of the element in the object element
     */
    void remove(const String& name);

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
     * @param name              Optional name of the element in the object element. Otherwise, use this element.
     */
    double getNumber(const String& name="") const;

    /**
     * Get value of JSON element
     * @param name              Optional name of the element in the object element. Otherwise, use this element.
     */
    String getString(const String& name="") const;

    /**
     * Get value of JSON element
     * @param name              Optional name of the element in the object element. Otherwise, use this element.
     */
    bool getBoolean(const String& name="") const;

    /**
     * Get value of JSON element.
     * If you want to modify elements of the array inside
     * this element, get array element using [name] and then add() or remove() its item(s).
     * Alternatively, create a new ArrayData object and replace existing one.
     * @param name              Optional name of the element in the object element. Otherwise, use this element.
     */
    ArrayData& getArray(const String& name="");

    /**
     * Get value of JSON element.
     * If you want to modify elements of the array inside
     * this element, get array element using [name] and then add() or remove() its item(s).
     * Alternatively, create a new ArrayData object and replace existing one.
     * @param name              Optional name of the element in the object element. Otherwise, use this element.
     */
    const ArrayData& getArray(const String& name="") const;

    /**
     * Get value of JSON element
     * If you want to modify elements of the object inside
     * this element, get object element using [name] and then add() or remove() its item(s).
     * Alternatively, create a new ObjectData object and replace existing one.
     * @param name              Optional name of the element in the object element. Otherwise, use this element.
     */
    ObjectData& getObject(const String& name="");

    /**
     * Get value of JSON element
     * If you want to modify elements of the object inside
     * this element, get object element using [name] and then add() or remove() its item(s).
     * Alternatively, create a new ObjectData object and replace existing one.
     * @param name              Optional name of the element in the object element. Otherwise, use this element.
     */
    const ObjectData& getObject(const String& name="") const;

    /**
     * Export JSON element (and all children) to stream
     * @param stream            Stream to export JSON
     * @param formatted         If true then JSON text is nicely formatted, but takes more space
     */
    void exportTo(std::ostream& stream, bool formatted=true) const;

    /**
     * Export JSON element (and all children) to XML element
     * @param name              Parent element name
     * @param parentNode        XML element to export JSON
     */
    void exportTo(const std::string& name, xml::Element& parentNode) const;

    /** @brief Selects elements as defined by XPath
     *
     * The implementation is just started, so only limited XPath standard part is supported.
     * Currently, examples 1 through 1 from http://www.zvon.org/xxl/XPathTutorial/Output/example1.html
     * are working fine with the exceptions:
     * - no functions are supported yet
     * - no attributes supported, because it isn't XML
     * @param elements          The resulting list of elements
     * @param xpath             The xpath for elements
     */
    void select(ElementSet& elements, const String& xpath);

    /**
     * Element type check
     * @return true if element is a number
     */
    bool isNumber()  const { return m_type == JDT_NUMBER; }

    /**
     * Conversion to integer
     */
    operator int () const { return (int) getNumber(); }

    /**
     * Conversion to double
     */
    operator double () const { return getNumber(); }

    /**
     * Element type check
     * @return true if element is a string
     */
    bool isString()  const { return m_type == JDT_STRING; }

    /**
     * Conversion to double
     */
    operator String () const { return getString(); }

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

    /**
     * Get number of elements in array or object.
     * Returns 0 for not { JDT_ARRAY, JDT_OBJECT }
     */
    size_t size() const;

    /**
     * Optimize arrays
     * Walks through the JSON document, and convert objects that contain
     * only single array field, to arrays - by removing unnecessary name.
     * @param name              Optional field name, use any name if empty string
     */
    void optimizeArrays(const std::string& name="item");

    /**
     * Get document this JSON element belongs to.
     * @return pointer to the owner document
     */
    Document* getDocument() const;
};

}}

#endif
