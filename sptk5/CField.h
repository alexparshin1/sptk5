/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CField.h - description                                 ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __CFIELDS_H__
#define __CFIELDS_H__

#include <sptk5/CBuffer.h>
#include <sptk5/CDateTime.h>
#include <sptk5/CVariant.h>
#include <sptk5/cxml>

#include <string>

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

class CQuery;

#include <sptk5/CVariant.h>

class CFieldList;

/// @brief Data field for CDataSource.
///
/// Contains field name, field type, field data and field format information.
class SP_EXPORT CField : public Variant
{
    friend class CFieldList;
protected:
    std::string m_name;         ///< Field name

public:
    std::string displayName;    ///< Optional display field name
public:
    /// @brief Constructor
    /// @param name const char *, field name
    CField(const char *name);

    /// @brief Combination of field view attributes
    struct {
        int     width:10;       ///< Field width
        int     precision:5;    ///< Field precision
        int     flags:16;       ///< Field flags like alignment, etc
        bool    visible:1;      ///< Is field visible?
    } view;

    /// @brief Returns field name
    const std::string& fieldName() const
    {
        return m_name;
    }

    /// @brief Sets the NULL state
    ///
    /// Useful for the database operations.
    /// Retains the data type. Sets the data to zero(s).
    /// @param vtype CVariantType, optional variant type to enforce
    virtual void setNull(VariantType vtype = VAR_NONE);

    /// @brief Assignment operation
    virtual CField& operator =(const Variant &C)
    {
        if (this == &C)
            return *this;

        setData(C);
        return *this;
    }

    /// @brief Assignment operation
    virtual CField& operator =(int64_t value)
    {
        setInt64(value);
        return *this;
    }

    /// @brief Assignment operation
    virtual CField& operator =(uint64_t value)
    {
        setInt64((int64_t) value);
        return *this;
    }

    /// @brief Assignment operation
    virtual CField& operator =(int32_t value)
    {
        setInteger(value);
        return *this;
    }

    /// @brief Assignment operation
    virtual CField& operator =(uint32_t value)
    {
        setInteger((int32_t) value);
        return *this;
    }

    /// @brief Assignment operation
    virtual CField& operator =(int16_t value)
    {
        setInteger(value);
        return *this;
    }

    /// @brief Assignment operation
    virtual CField& operator =(uint16_t value)
    {
        setInteger(value);
        return *this;
    }

    /// @brief Assignment operation
    virtual CField& operator =(float value)
    {
        setFloat(value);
        return *this;
    }

    /// @brief Assignment operation
    virtual CField& operator =(double value)
    {
        setFloat(value);
        return *this;
    }

    /// @brief Assignment operation
    virtual CField& operator =(const char * value)
    {
        setString(value);
        return *this;
    }

    /// @brief Assignment operation
    virtual CField& operator =(const std::string& value)
    {
        setString(value.c_str(), (uint32_t) value.length());
        return *this;
    }

    /// @brief Assignment operation
    virtual CField& operator =(CDateTime value)
    {
        setDateTime(value);
        return *this;
    }

    /// @brief Assignment operation
    virtual CField& operator =(const void *value)
    {
        setImagePtr(value);
        return *this;
    }

    /// @brief Assignment operation
    virtual CField& operator =(const CBuffer& value)
    {
        setBuffer(value.data(), value.bytes());
        return *this;
    }

    /// @brief Better (than in base class) conversion method
    virtual std::string asString() const THROWS_EXCEPTIONS;

    /// @brief Exports the field data into XML node
    ///
    /// If the compactXmlMode flag is true, the field is exported as an attribute.
    /// Otherwise, the field is exported as subnodes.
    /// For the fields of the VAR_TEXT type, the subnode is created containing CDATA section.
    /// @param node CXmlNode&, a node to export field data into
    /// @param compactXmlMode bool, compact XML mode flag
    void toXML(CXmlNode& node, bool compactXmlMode) const;
};
/// @}
}
#endif
