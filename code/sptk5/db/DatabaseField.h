/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/sptk.h>

#include <sptk5/Field.h>
#include <sptk5/Strings.h>
#include <sptk5/Variant.h>

namespace sptk {

/**
 * @addtogroup Database Database Support
 * @{
 */

/**
 * @brief database field
 *
 * A special variation of CField to support database field essentials
 */

class SP_EXPORT DatabaseField
    : public Field
{
    friend class Query;

public:
    /**
     * Constructor
     * @param fieldName			Field name
     * @param fieldColumn		Field column number
     * @param fieldType			Database field type
     * @param dataType			Variant data type
     * @param fieldLength		Database field length
     * @param fieldScale		Database field scale
     */
    DatabaseField(const String& fieldName, int fieldColumn, int fieldType, VariantDataType dataType, int fieldLength,
                  int fieldScale = 4);

    /**
     * @brief Checks the internal buffer size
     *
     * The internal buffer is automatically extended to fit the required size of data
     * @param sz				Data size (in bytes)
     */
    void checkSize(size_t sz);

    /**
     * @brief Sets the internal data size
     *
     * The internal buffer is not modified, only the data size is set.
     * @param sz				Data size (in bytes)
     */

    void setDataSize(size_t sz)
    {
        dataSize(sz);
    }

    /**
     * Reports field column number
     */

    int fieldColumn() const
    {
        return m_fldColumn;
    }

    /**
     * Reports database field type
     */

    int fieldType() const
    {
        return m_fldType;
    }

    /**
     * Reports field size
     */

    uint32_t fieldSize() const
    {
        return (uint32_t) m_fldSize;
    }

    /**
     * Column display format
     * @return Column display format
     */
    String displayFormat() const
    {
        return m_displayFormat;
    }

    /**
     * Set column display format
     */
    void displayFormat(const String& format)
    {
        m_displayFormat = format;
    }

    /**
     * Column alignment
     * @return Column alignment
     */
    int alignment() const
    {
        return m_alignment;
    }

    /**
     * Set column alignment
     */
    void alignment(int al)
    {
        m_alignment = al;
    }

protected:
    /**
     * Set field type
     * @param fieldType         Field type
     * @param fieldLength       Field length
     * @param fieldScale        Field scale
     */
    void setFieldType(int fieldType, int fieldLength, int fieldScale)
    {
        m_fldType = fieldType;
        m_fldSize = fieldLength;
        m_fldScale = fieldScale;
    }

    String doubleDataToString() const override;

private:
    int m_fldType;                ///< Native database data type
    int m_fldColumn;              ///< Field column number in recordset
    int m_fldSize;                ///< Field size
    int m_fldScale;               ///< Field scale, optional, for floating point fields
    String m_displayFormat;       ///< Column display format
    int m_alignment {ALIGN_LEFT}; ///< Column alignment
};

using SDatabaseField = std::shared_ptr<DatabaseField>;

/**
 * @}
 */
} // namespace sptk
