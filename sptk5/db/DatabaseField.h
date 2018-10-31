/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        DatabaseField.h - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
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

protected:

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


public:

    /**
     * Constructor
     * @param fieldName std::string, field name
     * @param fieldColumn int, field column number
     * @param fieldType int, database field type
     * @param dataType VariantType, variant data type
     * @param fieldLength int, database field length
     * @param fieldScale int, database field scale
     */
    DatabaseField(
            const String& fieldName, int fieldColumn, int fieldType, VariantType dataType, int fieldLength,
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
     * @param sz uint32_t, data size (in bytes)
     * @returns true if success
     */
    bool checkSize(uint32_t sz);

    /**
     * @brief Sets the internal data size
     *
     * The internal buffer is not modified, only the data size is set.
     * @param sz uint32_t, data size (in bytes)
     */

    void setDataSize(uint32_t sz)
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
