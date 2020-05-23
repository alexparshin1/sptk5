/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonElement.h - description                            ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 16 2013                                   ║
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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
#include <utility>

namespace sptk::json {

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

class SP_EXPORT ElementData
{
    friend class Document;
    friend class ArrayData;
    friend class ObjectData;
    friend class Parser;

    /**
     * Parent JSON document
     */
    Document*		m_document {nullptr};

    /**
     * Parent JSON element
     */
    Element*		m_parent {nullptr};

    /**
     * JSON element type
     */
    Type            m_type {JDT_NULL};

    /**
     * JSON element data
     */
    class Data
    {
        uint64_t m_storage {0};

        template <typename T> T& as() {
            auto* ptr = (void*) &m_storage;
            return *(T*) ptr;
        }

        template <typename T> const T& as() const {
            auto* ptr = (void*) &m_storage;
            return *(const T*) ptr;
        }

        template <typename T> void set(T& value) const {
            auto* ptr = (void*) &m_storage;
            *(T*) ptr = value;
        }

    public:
        double               get_number()  const { return as<double>(); }
        bool                 get_boolean() const { return as<bool>(); }
        const std::string*   get_string()  const { return as<std::string*>(); }
        ArrayData*           get_array()   const { return as<ArrayData*>(); }
        ObjectData*          get_object()  const { return as<ObjectData*>(); }
        ArrayData*           get_array()         { return as<ArrayData*>(); }
        ObjectData*          get_object()        { return as<ObjectData*>(); }

        void                 set_number(double number)          { set(number); }
        void                 set_boolean(bool boolean)          { set(boolean); }
        void                 set_string(const std::string* s)   { set(s); }
        void                 set_array(ArrayData* array)        { set(array); }
        void                 set_object(ObjectData* object)     { set(object); }
    };

    Data m_data;

protected:

    /**
     * XPath element
     */
    struct XPathElement
    {
        String          name;       ///< Path element name
        int             index {0};  ///< Path element index(position) from start: 1..N - element index, -1 - last element, 0 - don't use
        /**
         * Constructor
         * @param name          Path element name
         * @param index         Path element index(position) from start: 1..N - element index, -1 - last element, 0 - don't use
         */
        XPathElement(String  name, int index) : name(std::move(name)), index(index) {}

        /**
         * Copy constructor
         * @param other         Other object to copy from
         */
        XPathElement(const XPathElement& other) = default;
    };

    /**
     * XPath
     */
    struct XPath : std::vector<XPathElement>
    {
        bool rootOnly {false};          ///< XPath starts from root flag
        /**
         * Constructor
         * @param xpath           XPath expression
         */
        explicit XPath(const String& xpath);
    };

    [[nodiscard]] Data& data() { return m_data; }
    [[nodiscard]] const Data& data() const { return m_data; }

    /**
     * Parent JSON element
     */
    [[nodiscard]] Element* parent() { return m_parent; }

    void setDocument(Document* document) { m_document = document; }
    void setParent(Element* parent) { m_parent = parent; }
    void setType(Type type) { m_type = type; }

    /**
     * Get immediate child element, or return this element if the name is empty.
     * Throws an exception if child is not found.
     * @param name              Name of the element in the object element
     */
    [[nodiscard]] Element& getChild(const String& name);

    /**
     * Get immediate child element, or return this element if the name is empty.
     * Throws an exception if child is not found.
     * @param name              Name of the element in the object element
     */
    [[nodiscard]] const Element& getChild(const String& name) const;

    /**
     * Find child elements matching particular xpath element
     * @param elements          Elements matching xpath (output)
     * @param xpath             Xpath elements
     * @param xpathPosition     Position in xpath currently being checked
     * @param rootOnly          Flag indicating that only root level elements are checked
     */
    void selectChildElements(ElementSet& elements, const XPath& xpath, bool rootOnly) const;

    /**
     * Append matched element to element set
     * @param elements          Matched element set
     * @param xpathElement      Current XPath element
     * @param element           Current element
     */
    static void appendMatchedElement(ElementSet& elements, const ElementData::XPathElement& xpathElement, Element* element);

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
     * Constructor
     * @param document          Parent document
     * @param type              Data type
     */
    ElementData(Document* document, Type type) noexcept
    : m_document(document), m_parent(nullptr), m_type(type)
    {
    }

public:

    virtual ~ElementData() = default;

    /**
     * Parent JSON document
     */
    [[nodiscard]] Document* getDocument() { return m_document; }

    /**
     * Parent JSON document
     */
    [[nodiscard]] const Document* getDocument() const { return m_document; }

    /**
     * Clear JSON element.
     * Releases memory allocated by string, array, or object data,
     * and sets the type to JDT_NULL.
     */
    void clear();

    /**
     * Get number of elements in array or object.
     * Returns 0 for not { JDT_ARRAY, JDT_OBJECT }
     */
    [[nodiscard]] size_t size() const;

    /**
     * Element type check
     * @param type              Type or types bit combination
     * @return true if element is a number
     */
    [[nodiscard]] bool is(size_t type) const { return (m_type & type) != 0; }

    /**
     * JSON element type
     */
    [[nodiscard]] Type type() const { return m_type; }

    /**
     * Find elements matching particular xpath element
     * @param elements          Elements matching xpath (output)
     * @param xpath             Xpath elements
     * @param xpathPosition     Position in xpath currently being checked
     * @param rootOnly          Flag indicating that only root level elements are checked
     */
    void selectElements(ElementSet& elements, const XPath& xpath, size_t xpathPosition, bool rootOnly);

    /**
     * Remove JSON element by name from this JSON object element
     * @param name              Name of the element in the object element
     */
    void remove(const String& name);

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
     * Find JSON element in JSON object element
     * @param name              Name of the element in the object element
     * @returns Element for the name, or NULL if not found
     */
    [[nodiscard]] const Element* find(const String& name) const;

    /**
     * Find JSON element in JSON object element
     * @param name              Name of the element in the object element
     * @returns Element for the name, or NULL if not found
     */
    [[nodiscard]] Element* find(const String& name);
};

/**
 * XML Element getters
 */
class SP_EXPORT ElementGetMethods : public ElementData
{
protected:
    /**
     * Constructor
     * @param document          Parent document
     * @param type              Data type
     */
    ElementGetMethods(Document* document, Type type) noexcept
    : ElementData(document, type)
    {
    }

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

    /**
     * Export JSON array element to text format
     * @param stream            Output stream
     * @param formatted         If true then output JSON text is indented. Otherwise, it is using minimal formatting (elements separated with single space).
     * @param indent            Formatting indent, number of spaces
     * @param firstElement      First element indent, string of spaces
     * @param betweenElements   Space between elements, string of spaces
     * @param newLineChar       New line character(s)
     * @param indentSpaces      Indent, string of spaces
     */
    void exportArray(std::ostream& stream, bool formatted, size_t indent, const String& firstElement, const String& betweenElements, const String& newLineChar, const String& indentSpaces) const;

    /**
     * Export JSON object element to text format
     * @param stream            Output stream
     * @param formatted         If true then output JSON text is indented. Otherwise, it is using minimal formatting (elements separated with single space).
     * @param indent            Formatting indent, number of spaces
     * @param firstElement      First element indent, string of spaces
     * @param betweenElements   Space between elements, string of spaces
     * @param newLineChar       New line character(s)
     * @param indentSpaces      Indent, string of spaces
     */
    void exportObject(std::ostream& stream, bool formatted, size_t indent, const String& firstElement, const String& betweenElements, const String& newLineChar, const String& indentSpaces) const;

public:

    /**
     * Get value of JSON element
     * @param name              Optional name of the element in the object element. Otherwise, use this element.
     */
    [[nodiscard]] double getNumber(const String& name="") const;

    /**
     * Get value of JSON element
     * @param name              Optional name of the element in the object element. Otherwise, use this element.
     */
    [[nodiscard]] String getString(const String& name="") const;

    /**
     * Get value of JSON element
     * @param name              Optional name of the element in the object element. Otherwise, use this element.
     */
    [[nodiscard]] bool getBoolean(const String& name="") const;

    /**
     * Get value of JSON element.
     * If you want to modify elements of the array inside
     * this element, get array element using [name] and then add() or remove() its item(s).
     * Alternatively, create a new ArrayData object and replace existing one.
     * @param name              Optional name of the element in the object element. Otherwise, use this element.
     */
    [[nodiscard]] ArrayData& getArray(const String& name="");

    /**
     * Get value of JSON element.
     * If you want to modify elements of the array inside
     * this element, get array element using [name] and then add() or remove() its item(s).
     * Alternatively, create a new ArrayData object and replace existing one.
     * @param name              Optional name of the element in the object element. Otherwise, use this element.
     */
    [[nodiscard]] const ArrayData& getArray(const String& name="") const;

    /**
     * Get value of JSON element
     * If you want to modify elements of the object inside
     * this element, get object element using [name] and then add() or remove() its item(s).
     * Alternatively, create a new ObjectData object and replace existing one.
     * @param name              Optional name of the element in the object element. Otherwise, use this element.
     */
    [[nodiscard]] ObjectData& getObject(const String& name="");

    /**
     * Get value of JSON element
     * If you want to modify elements of the object inside
     * this element, get object element using [name] and then add() or remove() its item(s).
     * Alternatively, create a new ObjectData object and replace existing one.
     * @param name              Optional name of the element in the object element. Otherwise, use this element.
     */
    [[nodiscard]] const ObjectData& getObject(const String& name="") const;

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

    /**
     * Optimize arrays
     * Walks through the JSON document, and convert objects that contain
     * only single array field, to arrays - by removing unnecessary name.
     * @param name              Optional field name, use any name if empty string
     */
    void optimizeArrays(const std::string& name="item");

};

/**
 * JSON Element
 *
 * May contain any type of JSON object
 */
class SP_EXPORT Element : public ElementGetMethods
{
    friend class Document;
    friend class ArrayData;
    friend class ObjectData;

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
    explicit Element(Document* document) noexcept;

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
        Element element(getDocument(), other);
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
     * @return                  Created element
     */
    Element* set(const String& name)
    {
        return add(name, new Element(getDocument()));
    }

    /**
     * Set element in JSON object
     * @param name              Element name
     * @param value             Element value
     * @return                  Created element
     */
    template <typename T> Element* set(const String& name, T value)
    {
        return add(name, new Element(getDocument(), value));
    }

    /**
     * Add new empty element in JSON array
     * @return                  Created element
     */
    Element* push_back()
    {
        return add(new Element(getDocument()));
    }

    /**
     * Push element to JSON object
     * @param value             Element value
     * @return                  Created element
     */
    template <typename T> Element* push_back(T value)
    {
        return add(new Element(getDocument(), value));
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
    template <typename T>
    Element* add(const String& name, T value)
    {
        return add(name, new Element(getDocument(), value));
    }

public:

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
     * Conversion to integer
     */
    explicit operator int () const { return (int) getNumber(); }

    /**
     * Conversion to double
     */
    explicit operator double () const { return getNumber(); }

    /**
     * Conversion to double
     */
    explicit operator String () const { return getString(); }
};

/**
 * Escape special characters
 * @param text              Text with special characters
 * @returns escaped text
 */
std::string escape(const std::string& text);

/**
 * Decode escaped text
 * @param text              Escaped text
 * @returns decoded text
 */
std::string decode(const std::string& text);

}

#endif
