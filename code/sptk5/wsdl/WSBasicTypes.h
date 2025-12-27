/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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
        : WSType(name)
        , m_optional(optional)
    {
    }

    /**
     * @brief Copy constructor
     * @param other             Other object
     */
    WSBasicType(const WSBasicType& other) = default;

    /**
     * @brief Move constructor
     * @param other             Other object
     */
    WSBasicType(WSBasicType&& other) noexcept = default;

    /**
     * @brief Destructor
     */
    ~WSBasicType() override = default;

    /**
     * @brief Copy assignment
     * @param other             Other object
     * @return self
     */
    WSBasicType& operator=(const WSBasicType& other)
    {
        m_value = other.m_value;
        m_optional = other.m_optional;
        return *this;
    }

    /**
     * @brief Move assignment
     * @param other             Other object
     * @return self
     */
    WSBasicType& operator=(WSBasicType&& other) noexcept
    {
        m_value = std::move(other.m_value);
        m_optional = other.m_optional;
        return *this;
    }

    /**
     * @brief Return true if value is optional
     * @return true if value is optional
     */
    bool optional() const
    {
        return m_optional;
    }

    /**
     * @brief Sets optionality flag
     * @param opt               Element optionality flag
     */
    void optional(bool opt)
    {
        m_optional = opt;
    }

    /**
     * @brief Clears content (sets to NULL)
     */
    void clear() override
    {
        m_value.setNull(defaultDataType());
    }

    /**
     * @brief Loads type data from request XML node
     * @param attr              XML node
     * @param nullLargeData     If true, null large data
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
     * @brief Loads type data from string
     * @param attr              A string
     */
    virtual void load(const String& attr)
    {
        m_value.setString(attr);
    }

    /**
     * @brief Loads type data from database field
     * @param field             Database field
     */
    virtual void load(const Field& field)
    {
        m_value = *static_cast<const Variant*>(&field);
    }

    /**
     * @brief Returns element name
     */
    virtual operator String() const
    {
        return m_value.asString();
    }

    /**
     * @brief Conversion to string
     */
    String asString() const override
    {
        return m_value.asString();
    }

    /**
     * @brief Conversion to integer
     */
    auto asInteger() const
    {
        return value().asInteger();
    }

    /**
     * @brief Conversion to integer
     */
    auto asInt64() const
    {
        return value().asInt64();
    }

    /**
     * @brief Conversion to double
     */
    auto asFloat() const
    {
        return value().asFloat();
    }

    /**
     * @brief Conversion to boolean
     */
    auto asBool() const
    {
        return m_value.asBool();
    }

    /**
     * @brief Assign integer
     * @param _value            Value
     */
    void setInteger(int32_t _value)
    {
        m_value.setInteger(_value);
    }

    /**
     * @brief Assign integer
     * @param _value            Value
     */
    void setInt64(int64_t _value)
    {
        m_value.setInt64(_value);
    }

    /**
     * @brief Assign double
     * @param _value            Value
     */
    void setFloat(bool _value)
    {
        m_value.setFloat(_value);
    }

    /**
     * @brief Assign boolean
     * @param _value            Value
     */
    void setBool(bool _value)
    {
        m_value.setBool(_value);
    }

    /**
     * @brief Assign buffer
     * @param _value            Value
     */
    void setBuffer(const char* buffer, size_t size)
    {
        m_value.setBuffer((const uint8_t*) buffer, size, VariantDataType::VAR_BUFFER);
    }

    /**
     * @brief Check if value is null
     */
    bool isNull() const override
    {
        return m_value.isNull();
    }

    /**
     * Throw SOAPException is the object is null
     * @param parentTypeName    Parent object type name
     */
    void throwIfNull(const String& parentTypeName) const;

    /**
     * @brief Get data size
     */
    size_t dataSize() const
    {
        return m_value.dataSize();
    }

    /**
     * @brief Get data type
     */
    VariantDataType dataType() const
    {
        return m_value.dataType();
    }

    /**
     * @brief Set null
     */
    void setNull()
    {
        m_value.setNull(defaultDataType());
    }

    /**
     * @brief Get data as variant
     */
    Variant& value()
    {
        return m_value;
    }

    /**
     * @brief Get data as variant
     */
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
    /**
     * @brief Set null
     * @param dataType          Data type
     */
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
    Variant m_value;            ///< Data value
    bool    m_optional {false}; ///< Element optionality flag
};

/**
 * Base type for all standard WSDL types
 */
class SP_EXPORT WSString
    : public WSBasicType
{
public:
    /**
     * @brief Constructor
     */
    WSString()
    {
        setNull(VariantDataType::VAR_STRING);
    }

    /**
     * @brief Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    WSString(const String& name, bool optional)
        : WSBasicType(name.c_str(), optional)
    {
        setNull(VariantDataType::VAR_STRING);
    }

    /**
     * @brief Constructor
     * @param value             Value
     */
    explicit WSString(const String& _value)
    {
        value().setString(_value);
    }

    /**
     * @brief Return class name
     */
    String className() const override
    {
        return "WSString";
    }

    using WSBasicType::operator String;

    /**
     * @brief Default data type
     * @details Used in clear operations
     * @return  Default data type for the class
     */
    VariantDataType defaultDataType() const override
    {
        return VariantDataType::VAR_STRING;
    };

    /**
     * @brief Load data from XML node
     * @param attr              XML node
     * @param nullLargeData     Set null for elements with data size > 256 bytes
     */
    void load(const xdoc::SNode& attr, bool nullLargeData) override;

    /**
     * @brief Loads type data from string
     * @param attr              A string
     */
    void load(const String& attr) override;

    /**
     * @brief Loads type data from database field
     * @param field             Database field
     */
    void load(const Field& field) override;

    /**
     * @brief Assignment operation
     * @param _value            Value
     */
    WSString& operator=(const char* _value)
    {
        value().setString(_value);
        return *this;
    }

    /**
     * @brief Assignment operation
     * @param _value            Value
     */
    WSString& operator=(const String& _value)
    {
        value().setBuffer((const uint8_t*) _value.c_str(), _value.length(), VariantDataType::VAR_STRING);
        return *this;
    }

    /**
     * @brief Assignment operation
     * @param _value            Value
     */
    WSString& operator=(const Buffer& _value)
    {
        value().setBuffer(_value.data(), _value.bytes(), VariantDataType::VAR_BUFFER);
        return *this;
    }

    /**
     * @brief Assignment operation
     * @param _value            Value
     */
    WSString& operator=(int32_t _value)
    {
        value().setInteger(_value);
        return *this;
    }

    /**
     * @brief Assignment operation
     * @param _value            Value
     */
    WSString& operator=(int64_t _value)
    {
        value().setInt64(_value);
        return *this;
    }

    /**
     * @brief Get data as string
     * @return
     */
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

    using WSBasicType::operator String;

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

    using WSBasicType::operator String;

    /**
     * Return class name
     */
    String className() const override
    {
        return "WSDate";
    }

    /**
     * @brief Default data type
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
     * @brief Load data from string
     * @param attr              A string
     */
    void load(const String& attr) override;

    /**
     * @brief Load data from database field
     * @param field             Database field
     */
    void load(const Field& field) override;

    /**
     * @brief Assignment operation
     * @param _value            Value
     */
    WSDate& operator=(const DateTime& _value)
    {
        value().setDateTime(_value, true);
        return *this;
    }

    /**
     * @brief Conversion operator
     */
    auto asDate() const
    {
        return value().asDate();
    }

    /**
     * @brief Conversion operator
     */
    auto asDateTime() const
    {
        return value().asDateTime();
    }

    /**
     * @brief Conversion operator
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

    using WSBasicType::operator String;

    /**
     * @brief Return class name
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
     * @brief Load data from XML node
     * @param attr              XML node
     * @param nullLargeData     Set null for elements with data size > 256 bytes
     */
    void load(const xdoc::SNode& attr, bool nullLargeData) override;

    /**
     * @brief Load data from string
     * @param attr              A string
     */
    void load(const String& attr) override;

    /**
     * @brief Load data from database field
     * @param field             Database field
     */
    void load(const Field& field) override;

    /**
     * @brief Better (than in base class) conversion method
     */
    String asString() const override;

    /**
     * @brief Conversion operator
     */
    auto asDate() const
    {
        return value().asDate();
    }

    /**
     * @brief Conversion operator
     */
    auto asDateTime() const
    {
        return value().asDateTime();
    }

    /**
     * @brief Assignment operation
     */
    WSDateTime& operator=(const DateTime& _value)
    {
        value().setDateTime(_value);
        return *this;
    }

    /**
     * @brief Conversion operator
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
     * @brief Constructor
     */
    WSDouble()
    {
        setNull(VariantDataType::VAR_FLOAT);
    }

    /**
     * @brief Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    WSDouble(const String& name, bool optional)
        : WSBasicType(name.c_str(), optional)
    {
        setNull(VariantDataType::VAR_FLOAT);
    }

    /**
     * @brief Constructor
     * @param _value            Value
     */
    WSDouble(double _value)
    {
        value().setFloat(_value);
    }

    using WSBasicType::operator String;

    /**
     * @brief Return class name
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
     * @brief Load data from XML node
     * @param attr              XML node
     * @param nullLargeData     Set null for elements with data size > 256 bytes
     */
    void load(const xdoc::SNode& attr, bool nullLargeData) override;

    /**
     * @brief Load data from string
     * @param attr              A string
     */
    void load(const String& attr) override;

    /**
     * @brief Load data from database field
     * @param field             Database field
     */
    void load(const Field& field) override;

    /**
     * @brief Assignment operation
     * @param _value            Value
     */
    WSDouble& operator=(double _value)
    {
        value().setFloat(_value);
        return *this;
    }

    /**
     * @brief Conversion operator
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
     * @brief Constructor
     */
    WSInteger()
    {
        setNull(VariantDataType::VAR_INT);
    }

    /**
     * @brief Constructor
     * @param name              WSDL element name
     * @param optional          Element optionality flag
     */
    WSInteger(const String& name, bool optional)
        : WSBasicType(name.c_str(), optional)
    {
        setNull(VariantDataType::VAR_INT);
    }

    /**
     * @brief Constructor
     * @param value             Value
     */
    WSInteger(int _value)
    {
        value().setInteger(_value);
    }

    using WSBasicType::operator String;

    /**
     * @brief Return class name
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
     * @brief Load data from XML node
     * @param attr              XML node
     * @param nullLargeData     Set null for elements with data size > 256 bytes
     */
    void load(const xdoc::SNode& attr, bool nullLargeData) override;

    /**
     * @brief Load data from string
     * @param attr              A string
     */
    void load(const String& attr) override;

    /**
     * @brief Load data from database field
     * @param field             Database field
     */
    void load(const Field& field) override;

    /**
     * @brief Assignment operation
     * @param _value            Value
     */
    WSInteger& operator=(int64_t _value)
    {
        value().setInt64(_value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    WSInteger& operator=(int _value)
    {
        value().setInteger(_value);
        return *this;
    }

    /**
     * @brief Conversion operator
     */
    operator int32_t() const
    {
        return value().asInteger();
    }

    /**
     * @brief Conversion operator
     */
    operator int64_t() const
    {
        return value().asInt64();
    }

    /**
     * @brief Conversion operator
     */
    operator uint64_t() const
    {
        return static_cast<uint64_t>(value().asInt64());
    }
};

/**
 * @brief Converts type id name to WS type name
 * @param typeId                Type id name, returned by typeid(<type>).name()
 * @return WS type name without leading 'C' character
 */
String SP_EXPORT wsTypeIdToName(const String& typeIdName);

/**
 * @}
 */

} // namespace sptk
