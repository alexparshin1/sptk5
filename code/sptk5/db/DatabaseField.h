/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        DatabaseField.h - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __SPTK_DATABASEFIELD_H__
#define __SPTK_DATABASEFIELD_H__

#include <sptk5/sptk.h>

#include <sptk5/Variant.h>
#include <sptk5/Strings.h>
#include <sptk5/Field.h>

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

class SP_EXPORT DatabaseField : public Field
{
    friend class Query;

    /**
     * Native database data type
     */
    int     m_fldType;

    /**
     * Field column number in recordset
     */
    int     m_fldColumn;

    /**
     * Field size
     */
    int     m_fldSize;

    /**
     * Field scale, optional, for floating point fields
     */
    int     m_fldScale;

protected:

    void setFieldType(int fieldType, int fieldLength, int fieldScale)
    {
        m_fldType = fieldType;
        m_fldSize = fieldLength;
        m_fldScale = fieldScale;
    }

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
    DatabaseField(const String& fieldName, int fieldColumn, int fieldType, VariantType dataType, int fieldLength,
				  int fieldScale = 4);

    /**
     * Column display format
     */
    String          displayFormat;

    /**
     * Column alignment
     */
    int             alignment;

    /**
     * Is column visible?
     */
    bool            visible;


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
};
/**
 * @}
 */
}
#endif
