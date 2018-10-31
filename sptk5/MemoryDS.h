/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       MemoryDS.h - description                               ║
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

#ifndef __SPTK_MEMORYDS_H__
#define __SPTK_MEMORYDS_H__

#include <sptk5/sptk.h>
#include <sptk5/FieldList.h>
#include <sptk5/DataSource.h>
#include <sptk5/Exception.h>
#include <vector>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Base (memory) datasource
 *
 * Class CMemoryDS implements a special case of the datasource when the data
 * can be loaded all at once, in the datasource open() operation. It's a base class
 * for several actual datasources.
 */
class MemoryDS : public DataSource
{
protected:
    /**
     * Internal list of the dataset records
     */
    std::vector<FieldList *>  m_list;

    /**
     * Current record in the dataset.
     */
    FieldList*                m_current;

    /**
     * The index of the current record.
     */
    uint32_t                   m_currentIndex;

    /**
     * EOF flag for sequentual reading first(),next()..next().
     */
    bool                       m_eof;


protected:

    /**
     * Default constructor is protected, to prevent creating of the instance of that class
     */
    MemoryDS() : DataSource(), m_current(0L), m_currentIndex(0), m_eof(false) { }

    /**
     * Move constructor is protected, to prevent creating of the instance of that class
     */
    MemoryDS(MemoryDS&& other) noexcept
    : m_list(std::move(other.m_list)), m_current(other.m_current), m_currentIndex(other.m_currentIndex), m_eof(other.m_eof)
    {
        other.m_current = 0;
        other.m_currentIndex = 0;
        other.m_eof = false;
    }

public:

    /**
     * Destructor
     */
    virtual ~MemoryDS() {
        close();
    }

    /**
     * Clears all the records
     */
    virtual void clear();

    /**
     * Get current record
     * @return current record reference
     */
    virtual FieldList& current()
    {
        return *m_current;
    }

    /**
     * Field access by the field index, const version.
     * @param fieldIndex int, field index
     * @returns field reference
     */
    virtual const Field& operator [] (uint32_t fieldIndex) const;

    /**
     * Field access by the field index, non-const version.
     * @param fieldIndex int, field index
     * @returns field reference
     */
    virtual Field&       operator [] (uint32_t fieldIndex);

    /**
     * Field access by the field name, const version.
     * @param fieldName const char *, field name
     * @returns field reference
     */
    virtual const Field& operator [] (const char *fieldName) const;

    /**
     * Field access by the field name, non-const version.
     * @param fieldName const char *, field name
     * @returns field reference
     */
    virtual Field&       operator [] (const char *fieldName);

    /**
     * Returns user_data associated with the datasource.
     */
    virtual void             *user_data() const {
        return m_current->user_data();
    }

    /**
     * Returns field count in the datasource.
     * @returns field count
     */
    virtual uint32_t           fieldCount() const;

    /**
     * Returns record count in the datasource.
     * @returns record count
     */
    virtual uint32_t           recordCount() const;

    /**
     * Reads the field by name from the datasource.
     * @param fieldName const char *, field name
     * @param fieldValue CVariant, field value
     */
    virtual bool              readField(const char *fieldName,Variant& fieldValue);

    /**
     * Writes the field by name from the datasource.
     * @param fieldName const char *, field name
     * @param fieldValue CVariant, field value
     */
    virtual bool              writeField(const char *fieldName,const Variant& fieldValue);

    /**
     * Opens the datasource. Implemented in derved class.
     */
    virtual bool              open() {
        throw Exception("Not implemented yet");
    }

    /**
     * Closes the datasource.
     */
    virtual bool              close();

    /**
     * Moves to the first record of the datasource.
     */
    virtual bool              first();

    /**
     * Moves to the next record of the datasource.
     */
    virtual bool              next();

    /**
     * Moves to the prior record of the datasource.
     */
    virtual bool              prior();

    /**
     * Moves to the last record of the datasource.
     */
    virtual bool              last();

    /**
     * Finds the record by the record position (defined by record's user_data or key).
     */
    virtual bool              find(Variant position);

    /**
     * Returns true if there are no more records in the datasource. Implemented in derved class.
     */
    virtual bool              eof() const { return m_eof; }
};
/**
 * @}
 */
}
#endif
