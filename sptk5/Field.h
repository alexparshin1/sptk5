/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Field.h - description                                  ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_FIELD_H__
#define __SPTK_FIELD_H__

#include <sptk5/Buffer.h>
#include <sptk5/DateTime.h>
#include <sptk5/Variant.h>
#include <sptk5/cxml>

#include <string>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

class Query;

#include <sptk5/Variant.h>

class FieldList;

/**
 * @brief Data field for CDataSource.
 *
 * Contains field name, field type, field data and field format information.
 */
class SP_EXPORT Field : public Variant
{
    friend class FieldList;
protected:
    /**
     * Field name
     */
    std::string m_name;


public:
    /**
     * Optional display field name
     */
    std::string displayName;

public:
    /**
     * @brief Constructor
     * @param name const char *, field name
     */
    Field(const char *name);

    /**
     * @brief Combination of field view attributes
     */
    struct {
        /**
         * Field width
         */
        int     width:10;

        /**
         * Field precision
         */
        int     precision:5;

        /**
         * Field flags like alignment, etc
         */
        int     flags:16;

        /**
         * Is field visible?
         */
        bool    visible:1;

    } view;

    /**
     * @brief Returns field name
     */
    const std::string& fieldName() const
    {
        return m_name;
    }

    /**
     * @brief Sets the NULL state
     *
     * Useful for the database operations.
     * Retains the data type. Sets the data to zero(s).
     * @param vtype VariantType, optional variant type to enforce
     */
    void setNull(VariantType vtype) override;

    /**
     * @brief Assignment operation
     */
    virtual Field& operator =(const Variant &C)
    {
        if (this == &C)
            return *this;

        setData(C);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    virtual Field& operator =(int64_t value)
    {
        setInt64(value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    virtual Field& operator =(uint64_t value)
    {
        setInt64((int64_t) value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    virtual Field& operator =(int32_t value)
    {
        setInteger(value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    virtual Field& operator =(uint32_t value)
    {
        setInteger((int32_t) value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    virtual Field& operator =(int16_t value)
    {
        setInteger(value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    virtual Field& operator =(uint16_t value)
    {
        setInteger(value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    virtual Field& operator =(float value)
    {
        setFloat(value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    virtual Field& operator =(double value)
    {
        setFloat(value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    virtual Field& operator =(const char * value)
    {
        setString(value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    virtual Field& operator =(const std::string& value)
    {
        setString(value.c_str(), (uint32_t) value.length());
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    virtual Field& operator =(DateTime value)
    {
        setDateTime(value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    virtual Field& operator =(const void *value)
    {
        setImagePtr(value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    virtual Field& operator =(const Buffer& value)
    {
        setBuffer(value.data(), value.bytes());
        return *this;
    }

    /**
     * @brief Better (than in base class) conversion method
     */
    virtual std::string asString() const;

    /**
     * @brief Exports the field data into XML node
     *
     * If the compactXmlMode flag is true, the field is exported as an attribute.
     * Otherwise, the field is exported as subnodes.
     * For the fields of the VAR_TEXT type, the subnode is created containing CDATA section.
     * @param node XMLNode&, a node to export field data into
     * @param compactXmlMode bool, compact XML mode flag
     */
    void toXML(XMLNode& node, bool compactXmlMode) const;
};
/**
 * @}
 */
}
#endif
