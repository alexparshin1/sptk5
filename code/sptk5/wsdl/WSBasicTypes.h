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
#include <sptk5/wsdl/WSFieldIndex.h>
#include <sptk5/wsdl/WSType.h>
#include <sptk5/xdoc/Node.h>

namespace sptk {

/**
 * Base type for all standard WSDL types
 */
class SP_EXPORT WSBasicType
    : public WSType
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
        : m_name(name), m_optional(optional)
    {
    }

    WSBasicType(const WSBasicType& other) = default;

    WSBasicType(WSBasicType&& other) noexcept = default;

    virtual ~WSBasicType() noexcept = default;

    WSBasicType& operator=(const WSBasicType& other)
    {
        m_value = other.m_value;
        m_optional = other.m_optional;
        return *this;
    }

    WSBasicType& operator=(WSBasicType&& other) noexcept
    {
        m_value = std::move(other.m_value);
        m_optional = other.m_optional;
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
        m_value.setNull(defaultDataType());
    }

    /**
     * Loads type data from request XML node
     * @param attr              XML node
     */
    void load(const xdoc::SNode& attr, bool nullLargeData) override
    {
        if (nullLargeData && attr->getString().length() > 256)
        {
            m_value.setNull(VariantDataType::VAR_STRING);
        }
        else
        {
            m_value.load(attr);
        }
    }

    /**
     * Loads type data from string
     * @param attr              A string
     */
    virtual void load(const String& attr)
    {
        m_value.setString(attr);
    }

    /**
     * Loads type data from database field
     * @param field             Database field
     */
    virtual void load(const Field& field)
    {
        m_value = *static_cast<const Variant*>(&field);
    }

    /**
     * Returns element name
     */
    [[nodiscard]] String name() const override
    {
        return m_name;
    }

    /**
     * Conversion operator
     */
    virtual operator String() const
    {
        return m_value.asString();
    }

    /**
     * Conversion to string
     */
    String asString() const override
    {
        return m_value.asString();
    }

    auto asInteger() const
    {
        return value().asInteger();
    }

    auto asInt64() const
    {
        return value().asInt64();
    }

    auto asFloat() const
    {
        return value().asFloat();
    }

    auto asBool() const
    {
        return m_value.asBool();
    }

    void setInteger(int32_t _value)
    {
        m_value.setInteger(_value);
    }

    void setInt64(int64_t _value)
    {
        m_value.setInt64(_value);
    }

    void setFloat(bool _value)
    {
        m_value.setFloat(_value);
    }

    void setBool(bool _value)
    {
        m_value.setBool(_value);
    }

    void setBuffer(const char* buffer, size_t size)
    {
        m_value.setBuffer((const uint8_t*) buffer, size);
    }

    bool isNull() const override
    {
        return m_value.isNull();
    }

    /**
     * Throw SOAPException is the object is null
     * @param parentTypeName    Parent object type name
     */
    void throwIfNull(const String& parentTypeName) const;

    size_t dataSize() const
    {
        return m_value.dataSize();
    }

    VariantDataType dataType() const
    {
        return m_value.dataType();
    }

    void setNull()
    {
        m_value.setNull(defaultDataType());
    }

    Variant& value()
    {
        return m_value;
    }

    const Variant& value() const
    {
        return m_value;
    }

    /**
     * Adds an element to response XML with this object data
     * @param parent            Parent XML element
     * @param name              Optional name for child element
     */
    void exportTo(const xdoc::SNode& parent, const char* name = nullptr) const override;

protected:

    void setNull(VariantDataType dataType)
    {
        m_value.setNull(dataType);
    }

    /**
     * @brief   Default data type
     * @details Used in clear operations
     * @return  Default data type for the class
     */
    virtual VariantDataType defaultDataType() const
    {
        return VariantDataType::VAR_NONE;
    }

private:

    const String m_name;
    Variant m_value;
    bool m_optional {false};    ///< Element optionality flag
};

/**
 * Base type for all standard WSDL types
 */
class SP_EXPORT WSString
    : public WSBasicType
{
public:
    /**
     * Constructor
     */
    WSString()
    {
        setNull(VariantDataType::VAR_STRING);
    }

    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    WSString(const String& name, bool optional)
        : WSBasicType(name.c_str(), optional)
    {
        setNull(VariantDataType::VAR_STRING);
    }

    /**
     * Constructor
     * @param value             Value
     */
    WSString(const String& _value)
    {
        value().setString(_value);
    }

    /**
     * Return class name
     */
    String className() const override
    {
        return "WSString";
    }

    /**
     * @brief   Default data type
     * @details Used in clear operations
     * @return  Default data type for the class
     */
    VariantDataType defaultDataType() const override
    {
        return VariantDataType::VAR_STRING;
    };

    /**
     * Load data from XML node
     * @param attr              XML node
     * @param nullLargeData     Set null for elements with data size > 256 bytes
     */
    void load(const xdoc::SNode& attr, bool nullLargeData) override;

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
    WSString& operator=(const char* _value)
    {
        value().setString(_value);
        return *this;
    }

    /**
     * Assignment operation
     */
    WSString& operator=(const String& _value)
    {
        value().setBuffer((const uint8_t*) _value.c_str(), _value.length(), VariantDataType::VAR_STRING);
        return *this;
    }

    /**
     * Assignment operation
     */
    WSString& operator=(const Buffer& _value)
    {
        value().setBuffer(_value.data(), _value.bytes(), VariantDataType::VAR_BUFFER);
        return *this;
    }

    /**
     * Assignment operation
     */
    WSString& operator=(int32_t _value)
    {
        value().setInteger(_value);
        return *this;
    }

    /**
     * Assignment operation
     */
    WSString& operator=(int64_t _value)
    {
        value().setInt64(_value);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator String() const override
    {
        return value().asString();
    }

    const char* getString() const
    {
        return value().getString();
    }
};

/**
 * Wrapper for WSDL bool type
 */
class SP_EXPORT WSBool
    : public WSBasicType
{
public:
    /**
     * Constructor
     */
    WSBool()
    {
        setNull(VariantDataType::VAR_BOOL);
    }

    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    WSBool(const String& name, bool optional)
        : WSBasicType(name.c_str(), optional)
    {
        setNull(VariantDataType::VAR_BOOL);
    }

    /**
     * Constructor
     * @param value             Value
     * @param optional          Element optionality flag
     */
    explicit WSBool(bool _value)
    {
        value().setBool(_value);
    }

    /**
     * Return class name
     */
    String className() const override
    {
        return "WSBool";
    }

    /**
     * @brief   Default data type
     * @details Used in clear operations
     * @return  Default data type for the class
     */
    VariantDataType defaultDataType() const override
    {
        return VariantDataType::VAR_BOOL;
    };

    /**
     * Load data from XML node
     * @param attr              XML node
     * @param nullLargeData     Set null for elements with data size > 256 bytes
     */
    void load(const xdoc::SNode& attr, bool nullLargeData) override;

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
    virtual WSBool& operator=(bool _value)
    {
        value().setBool(_value);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator bool() const
    {
        return value().asBool();
    }
};

/**
 * Wrapper for WSDL date type
 */
class SP_EXPORT WSDate
    : public WSBasicType
{
public:
    /**
     * Constructor
     */
    WSDate()
    {
        setNull(VariantDataType::VAR_DATE);
    }

    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    WSDate(const String& name, bool optional)
        : WSBasicType(name.c_str(), optional)
    {
        setNull(VariantDataType::VAR_DATE);
    }

    /**
     * Constructor
     * @param value             Value
     */
    explicit WSDate(const DateTime& _value)
    {
        value().setDateTime(_value, true);
    }

    /**
     * Return class name
     */
    String className() const override
    {
        return "WSDate";
    }

    /**
     * @brief   Default data type
     * @details Used in clear operations
     * @return  Default data type for the class
     */
    VariantDataType defaultDataType() const override
    {
        return VariantDataType::VAR_DATE;
    };

    /**
     * Load data from XML node
     * @param attr              XML node
     * @param nullLargeData     Set null for elements with data size > 256 bytes
     */
    void load(const xdoc::SNode& attr, bool nullLargeData) override;

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
    WSDate& operator=(DateTime _value)
    {
        value().setDateTime(_value, true);
        return *this;
    }

    /**
     * Conversion operator
     */
    auto asDate() const
    {
        return value().asDate();
    }

    /**
     * Conversion operator
     */
    auto asDateTime() const
    {
        return value().asDateTime();
    }

    /**
     * Conversion operator
     */
    operator DateTime() const
    {
        return value().asDate();
    }
};

/**
 * Wrapper for WSDL dateTime type
 */
class SP_EXPORT WSDateTime
    : public WSBasicType
{
public:
    /**
     * Constructor
     */
    WSDateTime()
    {
        setNull(VariantDataType::VAR_DATE_TIME);
    }

    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    WSDateTime(const String& name, bool optional)
        : WSBasicType(name.c_str(), optional)
    {
        setNull(VariantDataType::VAR_DATE_TIME);
    }

    /**
     * Constructor
     * @param value             Value
     */
    explicit WSDateTime(const DateTime& _value)
    {
        value().setDateTime(_value);
    }

    /**
     * Return class name
     */
    String className() const override
    {
        return "WSDateTime";
    }

    /**
     * @brief   Default data type
     * @details Used in clear operations
     * @return  Default data type for the class
     */
    VariantDataType defaultDataType() const override
    {
        return VariantDataType::VAR_DATE_TIME;
    };

    /**
     * Load data from XML node
     * @param attr              XML node
     * @param nullLargeData     Set null for elements with data size > 256 bytes
     */
    void load(const xdoc::SNode& attr, bool nullLargeData) override;

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
        return value().asDate();
    }

    /**
     * Conversion operator
     */
    auto asDateTime() const
    {
        return value().asDateTime();
    }

    /**
     * Assignment operation
     */
    WSDateTime& operator=(DateTime _value)
    {
        value().setDateTime(_value);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator DateTime() const
    {
        return value().asDateTime();
    }
};

/**
 * Wrapper for WSDL double type
 */
class SP_EXPORT WSDouble
    : public WSBasicType
{
public:
    /**
     * Constructor
     */
    WSDouble()
    {
        setNull(VariantDataType::VAR_FLOAT);
    }

    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    WSDouble(const String& name, bool optional)
        : WSBasicType(name.c_str(), optional)
    {
        setNull(VariantDataType::VAR_FLOAT);
    }

    WSDouble(double _value)
    {
        value().setFloat(_value);
    }

    /**
     * Return class name
     */
    String className() const override
    {
        return "WSDouble";
    }

    /**
     * @brief   Default data type
     * @details Used in clear operations
     * @return  Default data type for the class
     */
    VariantDataType defaultDataType() const override
    {
        return VariantDataType::VAR_FLOAT;
    };

    /**
     * Load data from XML node
     * @param attr              XML node
     * @param nullLargeData     Set null for elements with data size > 256 bytes
     */
    void load(const xdoc::SNode& attr, bool nullLargeData) override;

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
    WSDouble& operator=(double _value)
    {
        value().setFloat(_value);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator double() const
    {
        return value().asFloat();
    }
};

/**
 * Wrapper for WSDL int type
 */
class SP_EXPORT WSInteger
    : public WSBasicType
{
public:
    /**
     * Constructor
     */
    WSInteger()
    {
        setNull(VariantDataType::VAR_INT);
    }

    /**
     * Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    WSInteger(const String& name, bool optional)
        : WSBasicType(name.c_str(), optional)
    {
        setNull(VariantDataType::VAR_INT);
    }

    /**
     * Constructor
     * @param value             Value
     */
    WSInteger(int _value)
    {
        value().setInteger(_value);
    }

    /**
     * Return class name
     */
    String className() const override
    {
        return "WSInteger";
    }

    /**
     * @brief   Default data type
     * @details Used in clear operations
     * @return  Default data type for the class
     */
    VariantDataType defaultDataType() const override
    {
        return VariantDataType::VAR_INT;
    };

    /**
     * Load data from XML node
     * @param attr              XML node
     * @param nullLargeData     Set null for elements with data size > 256 bytes
     */
    void load(const xdoc::SNode& attr, bool nullLargeData) override;

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
    WSInteger& operator=(int64_t _value)
    {
        value().setInt64(_value);
        return *this;
    }

    /**
     * Assignment operation
     */
    WSInteger& operator=(int _value)
    {
        value().setInteger(_value);
        return *this;
    }

    /**
     * Conversion operator
     */
    operator int32_t() const
    {
        return value().asInteger();
    }

    /**
     * Conversion operator
     */
    operator int64_t() const
    {
        return value().asInt64();
    }

    /**
     * Conversion operator
     */
    operator uint64_t() const
    {
        return (uint64_t) value().asInt64();
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
