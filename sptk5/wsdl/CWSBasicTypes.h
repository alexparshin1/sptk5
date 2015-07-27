/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CWSBasicTypes.h  -  description
                             -------------------
    begin                : 03 Aug 2012
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#ifndef __CWSBASICTYPES_H__
#define __CWSBASICTYPES_H__

#include <sptk5/cxml>
#include <sptk5/CField.h>
#include <sptk5/xml/CXmlElement.h>

namespace sptk {

/// @addtogroup wsdl WSDL-related Classes
/// @{

/// @brief Base type for all standard WSDL types
class WSBasicType : public CField
{
protected:
    bool m_optional;    ///< Element optionality flag

public:
    /// @brief Constructor
    /// @param name const char*, WSDL element name
    /// @param optional bool, Element optionality flag
    WSBasicType(const char* name, bool optional) : CField(name), m_optional(optional)
    {}

    /// @brief Sets optionality flag
    /// @param opt bool, Element optionality flag
    void optional(bool opt) { m_optional = opt; }

    /// @brief Clears content (sets to NULL)
    void clear() { setNull(); }

    /// @brief Loads type data from request XML node
    /// @param attr const CXmlNode*, XML node
    virtual void load(const CXmlNode* attr) = 0;

    /// @brief Loads type data from string
    /// @param attr std::string, A string
    virtual void load(std::string attr) = 0;

    /// @brief Adds an element to response XML with this object data
    /// @param parent CXmlElement*, Parent XML element
    CXmlElement* addElement(CXmlElement* parent) const;
};

/// @brief Base type for all standard WSDL types
class WSString : public WSBasicType
{
public:
    /// @brief Constructor
    /// @param name const char*, WSDL element name
    /// @param optional bool, Element optionality flag
    WSString(const char* name, bool optional=false) : WSBasicType(name, optional)
    {
        setNull(VAR_STRING);
    }

    /// @brief Loads type data from request XML node
    /// @param attr const CXmlNode*, XML node
    virtual void load(const CXmlNode* attr);

    /// @brief Loads type data from string
    /// @param attr std::string, A string
    virtual void load(std::string attr);

    /// @brief Assignment operation
    virtual WSString& operator =(const char * value)
    {
        setString(value);
        return *this;
    }

    /// @brief Assignment operation
    virtual WSString& operator =(const std::string& value)
    {
        setString(value.c_str(), (uint32_t) value.length());
        return *this;
    }

    /// @brief Assignment operation
    virtual WSString& operator =(const CBuffer& value)
    {
        setBuffer(value.data(), value.bytes());
        return *this;
    }

    /// @brief Conversion operator
    operator std::string() const THROWS_EXCEPTIONS
    {
        return asString();
    }
};

/// @brief Wrapper for WSDL bool type
class WSBool : public WSBasicType
{
public:
    /// @brief Constructor
    /// @param name const char*, WSDL element name
    /// @param optional bool, Element optionality flag
    WSBool(const char* name, bool optional=false) : WSBasicType(name, optional)
    {
        setNull(VAR_BOOL);
    }

    /// @brief Loads type data from request XML node
    /// @param attr const CXmlNode*, XML node
    virtual void load(const CXmlNode* attr);

    /// @brief Loads type data from string
    /// @param attr std::string, A string
    virtual void load(std::string attr);

    /// @brief Assignment operation
    virtual WSBool& operator =(bool value)
    {
        setBool(value);
        return *this;
    }

    /// @brief Conversion to bool
    bool asBool() const THROWS_EXCEPTIONS
    {
        return CField::asBool();
    }

    /// @brief Conversion operator
    operator std::string() const THROWS_EXCEPTIONS
    {
        return asString();
    }
};

/// @brief Wrapper for WSDL date type
class WSDate : public WSBasicType
{
public:
    /// @brief Constructor
    /// @param name const char*, WSDL element name
    /// @param optional bool, Element optionality flag
    WSDate(const char* name, bool optional=false) : WSBasicType(name, optional)
    {
        setNull(VAR_DATE);
    }

    /// @brief Loads type data from request XML node
    /// @param attr const CXmlNode*, XML node
    virtual void load(const CXmlNode* attr);

    /// @brief Loads type data from string
    /// @param attr std::string, A string
    virtual void load(std::string attr);

    /// @brief Assignment operation
    virtual WSDate& operator =(CDateTime value)
    {
        setDate(value);
        return *this;
    }

    /// @brief Conversion operator
    operator CDateTime() const THROWS_EXCEPTIONS
    {
        return asDate();
    }

    /// @brief Conversion operator
    operator std::string() const THROWS_EXCEPTIONS
    {
        return asString();
    }
};

/// @brief Wrapper for WSDL dateTime type
class WSDateTime : public WSBasicType
{
public:
    /// @brief Constructor
    /// @param name const char*, WSDL element name
    WSDateTime(const char* name, bool optional=false) : WSBasicType(name, optional)
    {
        setNull(VAR_DATE_TIME);
    }

    /// @brief Loads type data from request XML node
    /// @param attr const CXmlNode*, XML node
    virtual void load(const CXmlNode* attr);

    /// @brief Loads type data from string
    /// @param attr std::string, A string
    virtual void load(std::string attr);

    /// @brief Better (than in base class) conversion method
    virtual std::string asString() const THROWS_EXCEPTIONS;

    /// @brief Assignment operation
    virtual WSDateTime& operator =(CDateTime value)
    {
        setDateTime(value);
        return *this;
    }

    /// @brief Conversion operator
    operator CDateTime() const THROWS_EXCEPTIONS
    {
        return asDateTime();
    }

    /// @brief Conversion operator
    operator std::string() const THROWS_EXCEPTIONS
    {
        return asString();
    }
};

/// @brief Wrapper for WSDL double type
class WSDouble : public WSBasicType
{
public:
    /// @brief Constructor
    /// @param name const char*, WSDL element name
    /// @param optional bool, Element optionality flag
    WSDouble(const char* name, bool optional=false) : WSBasicType(name, optional)
    {
        setNull(VAR_FLOAT);
    }

    /// @brief Loads type data from request XML node
    /// @param attr const CXmlNode*, XML node
    virtual void load(const CXmlNode* attr);

    /// @brief Loads type data from string
    /// @param attr std::string, A string
    virtual void load(std::string attr);

    /// @brief Assignment operation
    virtual WSDouble& operator =(float value)
    {
        setFloat(value);
        return *this;
    }

    /// @brief Assignment operation
    virtual WSDouble& operator =(double value)
    {
        setFloat(value);
        return *this;
    }
    /// @brief Conversion operator
    operator float() const THROWS_EXCEPTIONS
    {
        return (float) asFloat();
    }

    /// @brief Conversion operator
    operator double() const THROWS_EXCEPTIONS
    {
        return asFloat();
    }

    /// @brief Conversion operator
    operator std::string() const THROWS_EXCEPTIONS
    {
        return asString();
    }
};

/// @brief Wrapper for WSDL int type
class WSInteger : public WSBasicType
{
public:
    /// @brief Constructor
    /// @param name const char*, WSDL element name
    /// @param optional bool, Element optionality flag
    WSInteger(const char* name, bool optional=false) : WSBasicType(name, optional)
    {
        setNull(VAR_INT);
    }

    /// @brief Loads type data from request XML node
    /// @param attr const CXmlNode*, XML node
    virtual void load(const CXmlNode* attr);

    /// @brief Loads type data from string
    /// @param attr std::string, A string
    virtual void load(std::string attr);

    /// @brief Assignment operation
    virtual WSInteger& operator =(int64_t value)
    {
        setInt64(value);
        return *this;
    }

    /// @brief Assignment operation
    virtual WSInteger& operator =(uint64_t value)
    {
        setInt64((int64_t) value);
        return *this;
    }

    /// @brief Assignment operation
    virtual WSInteger& operator =(int32_t value)
    {
        setInteger(value);
        return *this;
    }

    /// @brief Assignment operation
    virtual WSInteger& operator =(uint32_t value)
    {
        setInteger((int32_t) value);
        return *this;
    }

    /// @brief Assignment operation
    virtual WSInteger& operator =(int16_t value)
    {
        setInteger(value);
        return *this;
    }

    /// @brief Assignment operation
    virtual WSInteger& operator =(uint16_t value)
    {
        setInteger(value);
        return *this;
    }

    /// @brief Conversion operator
    operator int32_t() const THROWS_EXCEPTIONS
    {
        return asInteger();
    }

    /// @brief Conversion operator
    operator uint32_t() const THROWS_EXCEPTIONS
    {
        return (uint32_t) asInteger();
    }

    /// @brief Conversion operator
    operator int64_t() const THROWS_EXCEPTIONS
    {
        return asInt64();
    }

    /// @brief Conversion operator
    operator uint64_t() const THROWS_EXCEPTIONS
    {
        return (uint64_t) asInt64();
    }

    /// @brief Conversion operator
    operator std::string() const THROWS_EXCEPTIONS
    {
        return asString();
    }
};

/// @}

}
#endif
