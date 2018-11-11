/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       WSBasicTypes.h - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#ifndef __SPTK_WSBASICTYPES_H__
#define __SPTK_WSBASICTYPES_H__

#include <sptk5/cxml>
#include <sptk5/Field.h>
#include <sptk5/xml/Element.h>

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
    virtual sptk::String className() const = 0;
};

/**
 * Base type for all standard WSDL types
 */
class WSBasicType : public Field, public WSTypeName
{
protected:
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
    WSBasicType(const char* name, bool optional) : Field(name), m_optional(optional)
    {}

    /**
     * Sets optionality flag
     * @param opt               Element optionality flag
     */
    void optional(bool opt) { m_optional = opt; }

    /**
     * Clears content (sets to NULL)
     */
    void clear() { setNull(VAR_NONE); }

    /**
     * Loads type data from request XML node
     * @param attr              XML node
     */
    virtual void load(const xml::Node* attr) = 0;

    /**
     * Loads type data from string
     * @param attr              A string
     */
    virtual void load(const String& attr) = 0;

    /**
     * Loads type data from database field
     * @param field             Database field
     */
    virtual void load(const Field& field) = 0;

    /**
     * Adds an element to response XML with this object data
     * @param parent            Parent XML element
     */
    xml::Element* addElement(xml::Element* parent) const;

    /**
     * Returns element name
     */
    sptk::String name() const
    {
        return fieldName();
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
    WSString(const char* name, bool optional=false) : WSBasicType(name, optional)
    {
        setNull(VAR_STRING);
    }

    /**
     * Return class name
     */
    sptk::String className() const override { return "WSString"; }

    /**
     * Loads type data from request XML node
     * @param attr              XML node
     */
    virtual void load(const xml::Node* attr) override;

    /**
     * Loads type data from string
     * @param attr              A string
     */
    virtual void load(const String& attr) override;

    /**
     * Loads type data from database field
     * @param field             Database field
     */
    virtual void load(const Field& field) override;

    /**
     * Assignment operation
     */
    virtual WSString& operator =(const char * value) override
    {
        setString(value);
        return *this;
    }

    /**
     * Assignment operation
     */
    virtual WSString& operator =(const sptk::String& value) override
    {
        setString(value.c_str(), (uint32_t) value.length());
        return *this;
    }

    /**
     * Assignment operation
     */
    virtual WSString& operator =(const Buffer& value) override
    {
        setBuffer(value.data(), value.bytes());
        return *this;
    }

    /**
     * Assignment operation
     */
    virtual WSString& operator =(int32_t value) override
    {
        setInteger(value);
        return *this;
    }

    /**
     * Assignment operation
     */
    virtual WSString& operator =(int64_t value) override
    {
        setInt64(value);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator sptk::String() const
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
    WSBool(const char* name, bool optional=false) : WSBasicType(name, optional)
    {
        setNull(VAR_BOOL);
    }

    /**
     * Return class name
     */
    sptk::String className() const override { return "WSBool"; }

    /**
     * Loads type data from request XML node
     * @param attr              XML node
     */
    virtual void load(const xml::Node* attr) override;

    /**
     * Loads type data from string
     * @param attr              A string
     */
    virtual void load(const String& attr) override;

    /**
     * Loads type data from database field
     * @param field             Database field
     */
    virtual void load(const Field& field) override;

    /**
     * Assignment operation
     */
    virtual WSBool& operator =(bool value)
    {
        setBool(value);
        return *this;
    }

    /**
     * Conversion to bool
     */
    bool asBool() const
    {
        return Field::asBool();
    }

    /**
     * Conversion operator
     */
    operator sptk::String() const
    {
        return asString();
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
    WSDate(const char* name, bool optional=false) : WSBasicType(name, optional)
    {
        setNull(VAR_DATE);
    }

    /**
     * Return class name
     */
    sptk::String className() const override { return "WSDate"; }

    /**
     * Loads type data from request XML node
     * @param attr              XML node
     */
    virtual void load(const xml::Node* attr) override;

    /**
     * Loads type data from string
     * @param attr              A string
     */
    virtual void load(const String& attr) override;

    /**
     * Loads type data from database field
     * @param field             Database field
     */
    virtual void load(const Field& field) override;

    /**
     * Assignment operation
     */
    virtual WSDate& operator =(DateTime value) override
    {
        setDate(value);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator DateTime() const
    {
        return asDate();
    }

    /**
     * Conversion operator
     */
    operator sptk::String() const
    {
        return asString();
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
    WSDateTime(const char* name, bool optional=false) : WSBasicType(name, optional)
    {
        setNull(VAR_DATE_TIME);
    }

    /**
     * Return class name
     */
    sptk::String className() const override { return "WSDateTime"; }

    /**
     * Loads type data from request XML node
     * @param attr              XML node
     */
    virtual void load(const xml::Node* attr) override;

    /**
     * Loads type data from string
     * @param attr              A string
     */
    virtual void load(const sptk::String& attr) override;

    /**
     * Loads type data from database field
     * @param field             Database field
     */
    virtual void load(const Field& field) override;

    /**
     * Better (than in base class) conversion method
     */
    virtual sptk::String asString() const override;

    /**
     * Assignment operation
     */
    virtual WSDateTime& operator =(DateTime value) override
    {
        setDateTime(value);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator DateTime() const
    {
        return asDateTime();
    }

    /**
     * Conversion operator
     */
    operator sptk::String() const
    {
        return asString();
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
    WSDouble(const char* name, bool optional=false) : WSBasicType(name, optional)
    {
        setNull(VAR_FLOAT);
    }

    /**
     * Return class name
     */
    sptk::String className() const override { return "WSDouble"; }

    /**
     * Loads type data from request XML node
     * @param attr              XML node
     */
    virtual void load(const xml::Node* attr) override;

    /**
     * Loads type data from string
     * @param attr              A string
     */
    virtual void load(const sptk::String& attr) override;

    /**
     * Loads type data from database field
     * @param field             Database field
     */
    virtual void load(const Field& field) override;

    /**
     * Assignment operation
     */
    virtual WSDouble& operator =(float value) override
    {
        setFloat(value);
        return *this;
    }

    /**
     * Assignment operation
     */
    virtual WSDouble& operator =(double value) override
    {
        setFloat(value);
        return *this;
    }
    /**
     * Conversion operator
     */
    operator float() const
    {
        return (float) asFloat();
    }

    /**
     * Conversion operator
     */
    operator double() const
    {
        return asFloat();
    }

    /**
     * Conversion operator
     */
    operator sptk::String() const
    {
        return asString();
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
    WSInteger(const char* name, bool optional=false) : WSBasicType(name, optional)
    {
        setNull(VAR_INT);
    }

    /**
     * Return class name
     */
    sptk::String className() const override { return "WSInteger"; }

    /**
     * Loads type data from request XML node
     * @param attr              XML node
     */
    virtual void load(const xml::Node* attr) override;

    /**
     * Loads type data from string
     * @param attr              A string
     */
    virtual void load(const sptk::String& attr) override;

    /**
     * Loads type data from database field
     * @param field             Database field
     */
    virtual void load(const Field& field) override;

    /**
     * Assignment operation
     */
    virtual WSInteger& operator =(int64_t value) override
    {
        setInt64(value);
        return *this;
    }

    /**
     * Assignment operation
     */
    virtual WSInteger& operator =(uint64_t value) override
    {
        setInt64((int64_t) value);
        return *this;
    }

    /**
     * Assignment operation
     */
    virtual WSInteger& operator =(int32_t value) override
    {
        setInteger(value);
        return *this;
    }

    /**
     * Assignment operation
     */
    virtual WSInteger& operator =(uint32_t value) override
    {
        setInteger((int32_t) value);
        return *this;
    }

    /**
     * Assignment operation
     */
    virtual WSInteger& operator =(int16_t value) override
    {
        setInteger(value);
        return *this;
    }

    /**
     * Assignment operation
     */
    virtual WSInteger& operator =(uint16_t value) override
    {
        setInteger(value);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator int32_t() const
    {
        return asInteger();
    }

    /**
     * Conversion operator
     */
    operator uint32_t() const
    {
        return (uint32_t) asInteger();
    }

    /**
     * Conversion operator
     */
    operator int64_t() const
    {
        return asInt64();
    }

    /**
     * Conversion operator
     */
    operator uint64_t() const
    {
        return (uint64_t) asInt64();
    }

    /**
     * Conversion operator
     */
    operator sptk::String() const
    {
        return asString();
    }
};

/**
 * Wrapper for WSDL int type
 */
template <class T> class WSArray : public std::vector<T>, public WSTypeName
{
public:
    /**
     * Return class name
     */
    sptk::String className() const override { return "WSArray"; }
};

/**
 * @}
 */

}
#endif
