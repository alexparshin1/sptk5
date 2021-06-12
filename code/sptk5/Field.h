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
class FieldList;

/**
 * Data field for CDataSource.
 *
 * Contains field name, field type, field data and field format information.
 */
class SP_EXPORT Field : public Variant
{
    friend class FieldList;

public:
    /**
     * Combination of field view attributes
     */
    struct View {
        signed int  width:10;       ///< Field width
        unsigned    precision:5;    ///< Field precision
        unsigned    flags:16;       ///< Field flags like alignment, etc
        bool        visible:1;      ///< Is field visible?
    };

    /**
     * Constructor
     * @param name               Field name
     */
    explicit Field(const String& name);

    /**
     * Copy constructor
     * @param other              Other field object
     */
    Field(const Field& other) = default;

    /**
     * Move constructor
     * @param other              Other field object
     */
    Field(Field&& other) noexcept = default;

    ~Field() noexcept override = default;

    /**
     * Combination of field view attributes
     */
    View& view() { return m_view; };

    /**
     * Returns field name
     */
    const String& fieldName() const
    {
        return m_name;
    }

    /**
     * Sets the NULL state
     *
     * Useful for the database operations.
     * Retains the data type. Sets the data to zero(s).
     * @param vtype              Optional variant type to enforce
     */
    void setNull(VariantType vtype) override;

    /**
     * Copy assignment operation
     */
    Field& operator = (const Field& other)
    {
        if (this == &other)
            return *this;

        setData(other);
        m_name = other.m_name;
        m_displayName = other.m_displayName;

        return *this;
    }

    /**
     * Move assignment operation
     */
    Field& operator = (Field&& other) noexcept
    {
        if (this == &other)
            return *this;

        *(Variant*)this = std::move(other);
        m_name = std::move(other.m_name);
        m_displayName = std::move(other.m_displayName);

        return *this;
    }

    /**
     * Assignment operation
     */
    Field& operator = (const Variant &C)
    {
        if (this == &C)
            return *this;

        setData(C);
        return *this;
    }

    /**
     * Assignment operation
     */
    Field& operator =(int64_t value) override
    {
        setInt64(value);
        return *this;
    }

    /**
     * Assignment operation
     */
    Field& operator =(int32_t value) override
    {
        setInteger(value);
        return *this;
    }

    /**
     * Assignment operation
     */
    Field& operator =(double value) override
    {
        setFloat(value);
        return *this;
    }

    /**
     * Assignment operation
     */
    Field& operator =(const char * value) override
    {
        setString(value);
        return *this;
    }

    /**
     * Assignment operation
     */
    Field& operator =(const sptk::String& value) override
    {
        setBuffer(value.c_str(), value.length(), VAR_STRING);
        return *this;
    }

    /**
     * Assignment operation
     */
    Field& operator =(DateTime value) override
    {
        setDateTime(value);
        return *this;
    }

    /**
     * Assignment operation
     */
    Field& operator =(const void *value) override
    {
        setImagePtr(value);
        return *this;
    }

    /**
     * Assignment operation
     */
    Field& operator =(const Buffer& value) override
    {
        setBuffer(value.data(), value.bytes(), VAR_BUFFER);
        return *this;
    }

    /**
     * Better (than in base class) conversion method
     */
    String asString() const override;

    /**
     * Exports the field data into XML node
     *
     * If the compactXmlMode flag is true, the field is exported as an attribute.
     * Otherwise, the field is exported as subnodes.
     * For the fields of the VAR_TEXT type, the subnode is created containing CDATA section.
     * @param node               Node to export field data into
     * @param compactXmlMode     Compact XML mode flag
     */
    void toXML(xml::Node& node, bool compactXmlMode) const;

    String displayName() const
    {
        return m_displayName;
    }

    void displayName(const String& name)
    {
        m_displayName = name;
    }

private:

    String  m_name;          ///< Field name
    View    m_view {};       ///< Combination of field view attributes
    String  m_displayName;   ///< Optional display field name

    String doubleDataToString() const;
    String epochDataToDateTimeString() const;
};

typedef std::shared_ptr<Field> SField;

/**
 * @}
 */
}
