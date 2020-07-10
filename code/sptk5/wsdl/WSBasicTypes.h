/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
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

#ifndef __SPTK_WSBASICTYPES_H__
#define __SPTK_WSBASICTYPES_H__

#include <sptk5/cxml>
#include <sptk5/Field.h>
#include <sptk5/xml/Element.h>
#include <sptk5/json/JsonElement.h>

namespace sptk {

/**
 * @addtogroup wsdl WSDL-related Classes
 * @{
 */

/**
 * Class name support for WS-classes
 */
class SP_EXPORT WSTypeName
{
public:
    /**
     * Get WS type name
     * @return WS type name
     */
    virtual String className() const { return ""; }

    virtual void owaspCheck(const String& value);
};

/**
 * Base type for all standard WSDL types
 */
class SP_EXPORT WSBasicType : public Field, public WSTypeName
{
    /**
     * Element optionality flag
     */
    bool m_optional;

public:
    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    WSBasicType(const char* name, bool optional)
    : Field(name), m_optional(optional)
    {}

    WSBasicType(const WSBasicType& other)
    : Field(other), m_optional(other.m_optional)
    {}

    WSBasicType(WSBasicType&& other) noexcept
    : Field(std::move(other)), m_optional(std::exchange(other.m_optional, 0))
    {}

    WSBasicType& operator = (const WSBasicType& other)
    {
        if (&other != this) {
            Field::operator=(other);
            m_optional = other.m_optional;
        }
        return *this;
    }

    WSBasicType& operator = (WSBasicType&& other) noexcept
    {
        if (&other != this) {
            *(Field*) this = std::move(other);
            m_optional = other.m_optional;
        }
        return *this;
    }

    /**
     * Sets optionality flag
     * @param opt               Element optionality flag
     */
    void optional(bool opt)
    {
        m_optional = opt;
    }

    /**
     * Clears content (sets to NULL)
     */
    void clear()
    {
        setNull(VAR_NONE);
    }

    /**
     * Loads type data from request XML node
     * @param attr              XML node
     */
    void load(const xml::Node* attr) override
    {
        Variant::load(attr);
    }

    /**
     * Loads type data from request JSON element
     * @param attr              JSON element
     */
    void load(const json::Element* attr) override
    {
        Variant::load(attr);
    }

    /**
     * Loads type data from string
     * @param attr              A string
     */
    virtual void load(const String& attr)
    {
        setString(attr);
    }

    /**
     * Loads type data from database field
     * @param field             Database field
     */
    virtual void load(const Field& field)
    {
        *(Variant*) this = field;
    }

    /**
     * Adds an element to response XML with this object data
     * @param parent            Parent XML element
     * @param name              Optional name for child element
     */
    xml::Element* addElement(xml::Element* parent, const char* name=nullptr) const;

    /**
     * Adds an element to response JSON with this object data
     * @param parent            Parent JSON element
     */
    json::Element* addElement(json::Element* parent) const;

    /**
     * Returns element name
     */
    String name() const
    {
        return fieldName();
    }

    /**
     * Conversion operator
     */
    operator String() const override
    {
        return asString();
    }

    /**
     * Throw SOAPException is the object is null
     * @param parentTypeName    Parent object type name
     */
    void throwIfNull(const String& parentTypeName) const;
};

/**
 * Base type for all standard WSDL types
 */
class SP_EXPORT WSString : public WSBasicType
{
public:
    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    WSString(const char* name, bool optional = false)
            : WSBasicType(name, optional)
    {
        Field::setNull(VAR_STRING);
    }

    /**
     * Return class name
     */
    String className() const override
    {
        return "WSString";
    }

    /**
     * Load data from XML node
     * @param attr              XML node
     */
    void load(const xml::Node* attr) override;

    /**
     * Load data from JSON element
     * @param attr              JSON element
     */
    void load(const json::Element* attr) override;

    /**
     * Loads type data from string
     * @param attr              A string
     */
    void load(const String& attr) override;

    /**
     * Loads type data from database field
     * @param field             Database field
     */
    void load(const Field& field) override;

    /**
     * Assignment operation
     */
    WSString& operator=(const char* value) override
    {
        setString(value);
        return *this;
    }

    /**
     * Assignment operation
     */
    WSString& operator=(const String& value) override
    {
        setBuffer(value.c_str(), value.length(), VAR_STRING);
        return *this;
    }

    /**
     * Assignment operation
     */
    WSString& operator=(const Buffer& value) override
    {
        setBuffer(value.data(), value.bytes(), VAR_BUFFER);
        return *this;
    }

    /**
     * Assignment operation
     */
    WSString& operator=(int32_t value) override
    {
        setInteger(value);
        return *this;
    }

    /**
     * Assignment operation
     */
    WSString& operator=(int64_t value) override
    {
        setInt64(value);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator String() const override
    {
        return asString();
    }
};

/**
 * Wrapper for WSDL bool type
 */
class SP_EXPORT WSBool : public WSBasicType
{
public:
    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    explicit WSBool(const char* name, bool optional = false)
    : WSBasicType(name, optional)
    {
        Field::setNull(VAR_BOOL);
    }

    /**
     * Return class name
     */
    String className() const override
    {
        return "WSBool";
    }

    /**
     * Load data from XML node
     * @param attr              XML node
     */
    void load(const xml::Node* attr) override;

    /**
     * Load data from JSON element
     * @param attr              JSON element
     */
    void load(const json::Element* attr) override;

    /**
     * Load data from string
     * @param attr              A string
     */
    void load(const String& attr) override;

    /**
     * Loads type data from database field
     * @param field             Database field
     */
    void load(const Field& field) override;

    /**
     * Assignment operation
     */
    virtual WSBool& operator=(bool value)
    {
        setBool(value);
        return *this;
    }
};

/**
 * Wrapper for WSDL date type
 */
class SP_EXPORT WSDate : public WSBasicType
{
public:
    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    explicit WSDate(const char* name, bool optional = false)
    : WSBasicType(name, optional)
    {
        Field::setNull(VAR_DATE);
    }

    /**
     * Return class name
     */
    String className() const override
    {
        return "WSDate";
    }

    /**
     * Load data from XML node
     * @param attr              XML node
     */
    void load(const xml::Node* attr) override;

    /**
     * Load data from JSON element
     * @param attr              JSON element
     */
    void load(const json::Element* attr) override;

    /**
     * Load data from string
     * @param attr              A string
     */
    void load(const String& attr) override;

    /**
     * Load data from database field
     * @param field             Database field
     */
    void load(const Field& field) override;

    /**
     * Assignment operation
     */
    WSDate& operator=(DateTime value) override
    {
        setDateTime(value, true);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator DateTime() const override
    {
        return asDate();
    }
};

/**
 * Wrapper for WSDL dateTime type
 */
class SP_EXPORT WSDateTime : public WSBasicType
{
public:
    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    explicit WSDateTime(const char* name, bool optional = false)
    : WSBasicType(name, optional)
    {
        Field::setNull(VAR_DATE_TIME);
    }

    /**
     * Return class name
     */
    String className() const override
    {
        return "WSDateTime";
    }

    /**
     * Load data from XML node
     * @param attr              XML node
     */
    void load(const xml::Node* attr) override;

    /**
     * Load data from JSON element
     * @param attr              JSON element
     */
    void load(const json::Element* attr) override;

    /**
     * Load data from string
     * @param attr              A string
     */
    void load(const String& attr) override;

    /**
     * Load data from database field
     * @param field             Database field
     */
    void load(const Field& field) override;

    /**
     * Better (than in base class) conversion method
     */
    String asString() const override;

    /**
     * Assignment operation
     */
    WSDateTime& operator=(DateTime value) override
    {
        setDateTime(value);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator DateTime() const override
    {
        return asDateTime();
    }
};

/**
 * Wrapper for WSDL double type
 */
class SP_EXPORT WSDouble : public WSBasicType
{
public:
    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    explicit WSDouble(const char* name, bool optional = false)
            : WSBasicType(name, optional)
    {
        Field::setNull(VAR_FLOAT);
    }

    /**
     * Return class name
     */
    String className() const override
    { 
        return "WSDouble"; 
    }

    /**
     * Load data from XML node
     * @param attr              XML node
     */
    void load(const xml::Node* attr) override;

    /**
     * Load data from JSON element
     * @param attr              JSON element
     */
    void load(const json::Element* attr) override;

    /**
     * Load data from string
     * @param attr              A string
     */
    void load(const String& attr) override;

    /**
     * Load data from database field
     * @param field             Database field
     */
    void load(const Field& field) override;

    /**
     * Assignment operation
     */
    WSDouble& operator=(double value) override
    {
        setFloat(value);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator double() const override
    {
        return asFloat();
    }
};

/**
 * Wrapper for WSDL int type
 */
class SP_EXPORT WSInteger : public WSBasicType
{
public:
    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    explicit WSInteger(const char* name, bool optional = false)
    : WSBasicType(name, optional)
    {
        Field::setNull(VAR_INT);
    }

    /**
     * Return class name
     */
    String className() const override
    {
        return "WSInteger";
    }

    /**
     * Load data from XML node
     * @param attr              XML node
     */
    void load(const xml::Node* attr) override;

    /**
     * Load data from JSON node
     * @param attr              JSON node
     */
    void load(const json::Element* attr) override;

    /**
     * Load data from string
     * @param attr              A string
     */
    void load(const String& attr) override;

    /**
     * Load data from database field
     * @param field             Database field
     */
    void load(const Field& field) override;

    /**
     * Assignment operation
     */
    WSInteger& operator=(int64_t value) override
    {
        setInt64(value);
        return *this;
    }

    /**
     * Assignment operation
     */
    WSInteger& operator=(int32_t value) override
    {
        setInteger(value);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator int32_t() const override
    {
        return asInteger();
    }

    /**
     * Conversion operator
     */
    operator int64_t() const override
    {
        return asInt64();
    }

    /**
     * Conversion operator
     */
    operator uint64_t() const override
    {
        return (uint64_t) asInt64();
    }
};

/**
 * Wrapper for WSDL int type
 */
template<class T>
class SP_EXPORT WSArray : public std::vector<T>, public WSTypeName
{
public:
    /**
     * Default constructor
     */
    WSArray() {}

    /**
     * Copy constructor
     * @param other             Other object
     */
    WSArray(const WSArray<T>& other)
    : std::vector<T>(other)
    {
    }

    /**
     * Move constructor
     * @param other             Other object
     */
    WSArray(WSArray<T>&& other) noexcept
    : std::vector<T>(move(other))
    {
    }

    /**
     * Copy assignment
     * @param other             Other object
     * @return this object
     */
    WSArray& operator=(const WSArray<T>& other)
    {
        if (this != &other) {
            std::vector<T>::assign(other.begin(), other.end());
        }
        return *this;
    }

    /**
     * Move assignment
     * @param other             Other object
     * @return this object
     */
    WSArray& operator=(WSArray<T>&& other) noexcept
    {
        if (this != &other) {
            std::vector<T>::clear();
            std::vector<T>::swap(other);
        }
        return *this;
    }

    /**
     * Return class name
     */
    String className() const override
    {
        return "WSArray";
    }
};

/**
 * Converts type id name to WS type name
 * @param typeId                Type id name, returned by typeid(<type>).name()
 * @return WS type name without leading 'C' character
 */
String SP_EXPORT wsTypeIdToName(const String& typeIdName);

/**
 * @}
 */

}
#endif
