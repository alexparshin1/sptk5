/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSBasicTypes.h - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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
class WSTypeName
{
public:
    /**
     * Get WS type name
     * @return WS type name
     */
    virtual String className() const = 0;
};

/**
 * Base type for all standard WSDL types
 */
class WSBasicType : public Field, public WSTypeName
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
    void load(const xml::Node& attr) override
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
    virtual operator String() const
    {
        return asString();
    }
};

/**
 * Base type for all standard WSDL types
 */
class WSString : public WSBasicType
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
        setBuffer(value.data(), value.bytes(), VAR_BUFFER, false);
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
class WSBool : public WSBasicType
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
class WSDate : public WSBasicType
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
class WSDateTime : public WSBasicType
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
class WSDouble : public WSBasicType
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
class WSInteger : public WSBasicType
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
    operator uint32_t() const override
    {
        return (uint32_t) asInteger();
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
class WSArray : public std::vector<T>, public WSTypeName
{
public:
    /**
     * Return class name
     */
    String className() const override
    {
        return "WSArray";
    }
};

/**
 * @}
 */

}
#endif
