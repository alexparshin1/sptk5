/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Field.h - description                                  ║
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
    String m_name;


public:
    /**
     * Optional display field name
     */
    String displayName;

public:
    /**
     * @brief Constructor
     * @param name               Field name
     */
    explicit Field(const char *name);

    /**
     * @brief Copy constructor
     * @param other              Other field object
     */
    Field(const Field& other);

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
    const String& fieldName() const
    {
        return m_name;
    }

    /**
     * @brief Sets the NULL state
     *
     * Useful for the database operations.
     * Retains the data type. Sets the data to zero(s).
     * @param vtype              Optional variant type to enforce
     */
    void setNull(VariantType vtype) override;

    /**
     * @brief Assignment operation
     */
    Field& operator = (const Variant &C)
    {
        if (this == &C)
            return *this;

        setData(C);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    Field& operator =(int64_t value) override
    {
        setInt64(value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    Field& operator =(uint64_t value) override
    {
        setInt64((int64_t) value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    Field& operator =(int32_t value) override
    {
        setInteger(value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    Field& operator =(uint32_t value) override
    {
        setInteger((int32_t) value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    Field& operator =(int16_t value) override
    {
        setInteger(value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    Field& operator =(uint16_t value) override
    {
        setInteger(value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    Field& operator =(float value) override
    {
        setFloat(value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    Field& operator =(double value) override
    {
        setFloat(value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    Field& operator =(const char * value) override
    {
        setString(value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    Field& operator =(const sptk::String& value) override
    {
        setString(value.c_str(), (uint32_t) value.length());
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    Field& operator =(DateTime value) override
    {
        setDateTime(value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    Field& operator =(const void *value) override
    {
        setImagePtr(value);
        return *this;
    }

    /**
     * @brief Assignment operation
     */
    Field& operator =(const Buffer& value) override
    {
        setBuffer(value.data(), value.bytes());
        return *this;
    }

    /**
     * @brief Better (than in base class) conversion method
     */
    String asString() const override;

    /**
     * @brief Exports the field data into XML node
     *
     * If the compactXmlMode flag is true, the field is exported as an attribute.
     * Otherwise, the field is exported as subnodes.
     * For the fields of the VAR_TEXT type, the subnode is created containing CDATA section.
     * @param node               Node to export field data into
     * @param compactXmlMode     Compact XML mode flag
     */
    void toXML(xml::Node& node, bool compactXmlMode) const;
};
/**
 * @}
 */
}
#endif
