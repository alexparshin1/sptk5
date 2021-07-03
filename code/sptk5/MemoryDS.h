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

#include <sptk5/sptk.h>
#include <sptk5/FieldList.h>
#include <sptk5/DataSource.h>
#include <sptk5/Exception.h>
#include <mutex>
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
class SP_EXPORT MemoryDS
    : public DataSource
{
public:
    /**
     * Default constructor
     */
    MemoryDS() = default;

    /**
     * Deleted copy constructor
     * @param other             Other object
     */
    MemoryDS(const MemoryDS& other) = delete;

    /**
     * Move constructor
     * @param other             Other object
     */
    MemoryDS(MemoryDS&& other) noexcept
        : m_list(std::move(other.m_list)), m_current(std::move(other.m_current))
    {
    }

    ~MemoryDS() override = default;

    /**
     * Deleted copy assignment
     * @param other             Other object
     */
    MemoryDS& operator=(const MemoryDS& other) = delete;

    /**
     * Move assignment
     * @param other             Other object
     */
    MemoryDS& operator=(MemoryDS&& other) noexcept
    {
        std::scoped_lock lock(m_mutex, other.m_mutex);
        m_list = std::move(other.m_list);
        m_current = std::move(other.m_current);
        return *this;
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
        std::lock_guard lock(m_mutex);
        return *m_current;
    }

    /**
     * Field access by the field index, const version.
     * @param fieldIndex int, field index
     * @returns field reference
     */
    Field& operator[](size_t fieldIndex) override;

    /**
     * Field access by the field name, non-const version.
     * @param fieldName const char *, field name
     * @returns field reference
     */
    Field& operator[](const String& fieldName) override;

    /**
     * Returns field count in the datasource.
     * @returns field count
     */
    size_t fieldCount() const override;

    /**
     * Returns record count in the datasource.
     * @returns record count
     */
    size_t recordCount() const override;

    /**
     * Reads the field by name from the datasource.
     * @param fieldName const char *, field name
     * @param fieldValue CVariant, field value
     */
    bool readField(const char* fieldName, Variant& fieldValue) override;

    /**
     * Writes the field by name from the datasource.
     * @param fieldName const char *, field name
     * @param fieldValue CVariant, field value
     */
    bool writeField(const char* fieldName, const Variant& fieldValue) override;

    /**
     * Opens the datasource. Implemented in derved class.
     */
    bool open() override;

    /**
     * Closes the datasource.
     */
    bool close() override;

    /**
     * Moves to the first record of the datasource.
     */
    bool first() override;

    /**
     * Moves to the next record of the datasource.
     */
    bool next() override;

    /**
     * Moves to the prior record of the datasource.
     */
    bool prior() override;

    /**
     * Moves to the last record of the datasource.
     */
    bool last() override;

    /**
     * Finds the record by the record position (defined by record's user_data or key).
     */
    bool find(const String& fieldName, const Variant& position) override;

    /**
     * Returns true if there are no more records in the datasource. Implemented in derved class.
     */
    bool eof() const override
    {
        std::scoped_lock lock(m_mutex);
        return m_current == m_list.end();
    }

    bool empty() const;

    std::vector<FieldList>& rows()
    {
        std::scoped_lock lock(m_mutex);
        return m_list;
    }

    const std::vector<FieldList>& rows() const
    {
        std::scoped_lock lock(m_mutex);
        return m_list;
    }

    /**
     * Push back field list.
     * Memory DS takes ownership of the data
     * @param fieldList         Field list
     */
    void push_back(FieldList&& fieldList);

private:

    mutable std::mutex m_mutex;
    std::vector<FieldList> m_list;               // List of the dataset records
    std::vector<FieldList>::iterator m_current;  // DS iterator
};
/**
 * @}
 */
}
