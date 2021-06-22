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

#pragma once

#include <sptk5/Field.h>
#include <sptk5/cxml>
#include <sptk5/json/JsonElement.h>
#include <sptk5/wsdl/WSFieldIndex.h>
#include <sptk5/wsdl/WSType.h>
#include <sptk5/xml/Element.h>

namespace sptk {

/**
 * Base type for all standard WSDL types
 */
class SP_EXPORT WSBasicType : public WSType
{
public:
    /**
     * Constructor
     */
    WSBasicType() = default;

    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    WSBasicType(const char* name, bool optional)
    : m_field(name), m_optional(optional)
    {}

    WSBasicType(const WSBasicType& other)
    : m_field(other.m_field), m_optional(other.m_optional)
    {}

    WSBasicType(WSBasicType&& other) noexcept
    : m_field(std::move(other.m_field)), m_optional(std::exchange(other.m_optional, 0))
    {}

    virtual ~WSBasicType() noexcept = default;

    WSBasicType& operator = (const WSBasicType& other)
    {
        if (&other != this) {
            m_field = other.m_field;
            m_optional = other.m_optional;
        }
        return *this;
    }

    WSBasicType& operator = (WSBasicType&& other) noexcept
    {
        if (&other != this) {
            m_optional = other.m_optional;
            m_field = std::move(other.m_field);
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
    void clear() override
    {
        m_field.setNull(VAR_NONE);
    }

    /**
     * Loads type data from request XML node
     * @param attr              XML node
     */
    void load(const xml::Node* attr) override
    {
        static_cast<Variant*>(&m_field)->load(attr);
    }

    /**
     * Loads type data from request JSON element
     * @param attr              JSON element
     */
    void load(const json::Element* attr) override
    {
        static_cast<Variant*>(&m_field)->load(attr);
    }

    /**
     * Loads type data from string
     * @param attr              A string
     */
    virtual void load(const String& attr)
    {
        m_field.setString(attr);
    }

    /**
     * Loads type data from database field
     * @param field             Database field
     */
    virtual void load(const Field& field)
    {
        *static_cast<Variant*>(&m_field) = *static_cast<const Variant*>(&field);
    }

    /**
     * Returns element name
     */
    [[nodiscard]] String name() const override
    {
        return m_field.fieldName();
    }

    /**
     * Conversion operator
     */
    virtual operator String() const
    {
        return m_field.asString();
    }

    /**
     * Conversion to string
     */
    String asString() const override
    {
        return m_field.asString();
    }

    auto asInteger() const { return field().asInteger(); }
    auto asInt64() const { return field().asInt64(); }
    auto asFloat() const { return field().asFloat(); }
    auto asBool() const { return field().asBool(); }

    void setInteger(int32_t value) { field().setInteger(value); }
    void setInt64(int64_t value) { field().setInt64(value); }
    void setFloat(bool value) { field().setFloat(value); }
    void setBool(bool value) { field().setBool(value); }
    void setBuffer(const char* buffer, size_t size) { field().setBuffer((const uint8_t*) buffer, size); }

    bool isNull() const override
    {
        return m_field.isNull();
    }

    /**
     * Throw SOAPException is the object is null
     * @param parentTypeName    Parent object type name
     */
    void throwIfNull(const String& parentTypeName) const;

    VariantType dataType() const
    {
        return m_field.dataType();
    }

    void setNull(VariantType type)
    {
        m_field.setNull(type);
    }

    Field& field() { return m_field; }
    const Field& field() const { return m_field; }

    /**
     * Adds an element to response XML with this object data
     * @param parent            Parent XML element
     * @param name              Optional name for child element
     */
    void addElement(xml::Node* parent, const char* name=nullptr) const override;

    /**
     * Adds an element to response JSON with this object data
     * @param parent            Parent JSON element
     */
    void addElement(json::Element* parent) const override;

private:

    Field   m_field {""};
    bool    m_optional {false};    ///< Element optionality flag
};

/**
 * Base type for all standard WSDL types
 */
class SP_EXPORT WSString : public WSBasicType
{
public:
    /**
     * Constructor
     */
    WSString()
    {
        setNull(VAR_STRING);
    }

    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    WSString(const String& name, bool optional)
    : WSBasicType(name.c_str(), optional)
    {
        setNull(VAR_STRING);
    }

    /**
     * Constructor
     * @param value             Value
     */
    WSString(const String& value)
    {
        field().setString(value);
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
    WSString& operator=(const char* value)
    {
        field().setString(value);
        return *this;
    }

    /**
     * Assignment operation
     */
    WSString& operator=(const String& value)
    {
        field().setBuffer((const uint8_t*) value.c_str(), value.length(), VAR_STRING);
        return *this;
    }

    /**
     * Assignment operation
     */
    WSString& operator=(const Buffer& value)
    {
        field().setBuffer(value.data(), value.bytes(), VAR_BUFFER);
        return *this;
    }

    /**
     * Assignment operation
     */
    WSString& operator=(int32_t value)
    {
        field().setInteger(value);
        return *this;
    }

    /**
     * Assignment operation
     */
    WSString& operator=(int64_t value)
    {
        field().setInt64(value);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator String() const override
    {
        return field().asString();
    }

    const char* getString() const
    {
        return field().getString();
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
     */
    WSBool()
    {
        setNull(VAR_BOOL);
    }

    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    WSBool(const String& name, bool optional)
            : WSBasicType(name.c_str(), optional)
    {
        setNull(VAR_BOOL);
    }

    /**
     * Constructor
     * @param value             Value
     * @param optional          Element optionality flag
     */
    explicit WSBool(bool value)
    {
        field().setBool(value);
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
        field().setBool(value);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator bool() const
    {
        return field().asBool();
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
     */
    WSDate()
    {
        setNull(VAR_DATE);
    }

    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    WSDate(const String& name, bool optional)
    : WSBasicType(name.c_str(), optional)
    {
        setNull(VAR_DATE);
    }

    /**
     * Constructor
     * @param value             Value
     */
    explicit WSDate(const DateTime& value)
    {
        field().setDateTime(value, true);
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
    WSDate& operator=(DateTime value)
    {
        field().setDateTime(value, true);
        return *this;
    }

    /**
     * Conversion operator
     */
    auto asDate() const
    {
        return field().asDate();
    }

    /**
     * Conversion operator
     */
    auto asDateTime() const
    {
        return field().asDateTime();
    }

    /**
     * Conversion operator
     */
    operator DateTime() const
    {
        return field().asDate();
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
     */
    WSDateTime()
    {
        setNull(VAR_DATE_TIME);
    }

    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    WSDateTime(const String& name, bool optional)
    : WSBasicType(name.c_str(), optional)
    {
        setNull(VAR_DATE_TIME);
    }

    /**
     * Constructor
     * @param value             Value
     */
    explicit WSDateTime(const DateTime& value)
    {
        field().setDateTime(value);
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
     * Conversion operator
     */
    auto asDate() const
    {
        return field().asDate();
    }

    /**
     * Conversion operator
     */
    auto asDateTime() const
    {
        return field().asDateTime();
    }

    /**
     * Assignment operation
     */
    WSDateTime& operator=(DateTime value)
    {
        field().setDateTime(value);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator DateTime() const
    {
        return field().asDateTime();
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
     */
    WSDouble()
    {
        setNull(VAR_FLOAT);
    }

    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    WSDouble(const String& name, bool optional)
    : WSBasicType(name.c_str(), optional)
    {
        setNull(VAR_FLOAT);
    }

    WSDouble(double value)
    {
        field().setFloat(value);
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
    WSDouble& operator=(double value)
    {
        field().setFloat(value);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator double() const
    {
        return field().asFloat();
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
     */
    WSInteger()
    {
        setNull(VAR_INT);
    }

    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    WSInteger(const String& name, bool optional)
    : WSBasicType(name.c_str(), optional)
    {
        setNull(VAR_INT);
    }

    /**
     * Constructor
     * @param value             Value
     */
    WSInteger(int value)
    {
        field().setInteger(value);
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
    WSInteger& operator=(int64_t value)
    {
        field().setInt64(value);
        return *this;
    }

    /**
     * Assignment operation
     */
    WSInteger& operator=(int value)
    {
        field().setInteger(value);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator int32_t() const
    {
        return field().asInteger();
    }

    /**
     * Conversion operator
     */
    operator int64_t() const
    {
        return field().asInt64();
    }

    /**
     * Conversion operator
     */
    operator uint64_t() const
    {
        return (uint64_t) field().asInt64();
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
