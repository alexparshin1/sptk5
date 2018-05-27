/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CListViewRows.h - description                          ║
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

#ifndef __CLISTVIEWROWS_H__
#define __CLISTVIEWROWS_H__

#include <sptk5/Variant.h>
#include <sptk5/PackedStrings.h>
#include <sptk5/IntList.h>
#include <sptk5/Variant.h>

#include <vector>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

class CListView;

/**
 * The list of CPackedStrings * used inside CListView as the list of rows.
 */

class SP_EXPORT CListViewRows
{
    typedef CPackedStrings * PPackedStrings;
    typedef std::vector<PPackedStrings> CPSVector;
    friend class CListView;
private:
    /**
     * The sort column number
     */
    int32_t m_sortColumn;

    /**
     * The type of the sort column - determines the sort algorithm
     */
    VariantType m_sortColumnType;

    /**
     * Sort direction - ascending/descending
     */
    bool m_sortAscending;

protected:
    /**
     * The summary height of the rows
     */
    uint32_t m_fullHeight;

    /**
     * The list of the rows
     */
    CPSVector m_rows;


    /**
     * The sort column number for the sort functions
     */
    static int currentSortColumn;

    /**
     * The string data compare function
     */
    static bool compare_strings(const PPackedStrings&, const PPackedStrings&);

    /**
     * The integer data compare function
     */
    static bool compare_integers(const PPackedStrings&, const PPackedStrings&);

    /**
     * The float data compare function
     */
    static bool compare_floats(const PPackedStrings&, const PPackedStrings&);

    /**
     * The date data compare function
     */
    static bool compare_dates(const PPackedStrings&, const PPackedStrings&);

    /**
     * The date and time data compare function
     */
    static bool compare_datetimes(const PPackedStrings&, const PPackedStrings&);
public:

    /**
     * Default constructor
     */
    CListViewRows();

    /**
     * Destructor
     */
    ~CListViewRows();

    /**
     * destroys all the rows and removes them from the list
     */
    void clear();

    /**
     * Truncates a row list
     * @param count uint32_t, the maximum list size
     */
    void truncate(uint32_t count);

    /**
     * Removes a row
     * @param index uint32_t, row index
     */
    void remove(uint32_t index);

    /**
     * Adds the new row.
     * @param row CPackedStrings *, the new row pointer
     * @returns row index
     */
    uint32_t add
    (CPackedStrings *row);

    /**
     * Inserts the new row.
     * @param index uint32_t, insert position
     * @param row CPackedStrings *, the new row pointer
     * @returns row index
     */
    uint32_t insert(uint32_t index, CPackedStrings *row);

    /**
     * Updates the row by replacing it with a new one.
     * @param index uint32_t, row index
     * @param row CPackedStrings *, the new row pointer
     * @returns row index
     */
    uint32_t update(uint32_t index, CPackedStrings *row);

    /**
     * Row access by row index
     * @param index uint32_t, row index
     * @returns row pointer, or 0L if index is out of range
     */
    CPackedStrings* operator [] (uint32_t index) const
    {
        if (index < m_rows.size())
            return(CPackedStrings *) m_rows[index];
        return 0L;
    }

    /**
     * Finds the row
     * @param row CPackedStrings *, the row pointer
     * @returns the row index, or -1 if not found
     */
    int32_t indexOf(CPackedStrings *row) const;

    /**
     * Returns the row count
     */

    uint32_t size() const
    {
        return(uint32_t) m_rows.size();
    }

    /**
     * Sorts rows
     */
    void sort();

    /**
     * Returns the sort column number
     */

    int32_t sortColumn() const
    {
        return m_sortColumn;
    }

    /**
     * Sets the sort column
     * @param column int, sort column number (-1 means 'unsorted')
     * @param ctype VariantType, the sort column data type
     * @param sortNow bool, true if you want to sort immediatedly
     */
    void sortColumn(int column, VariantType ctype, bool sortNow);

    /**
     * Returns the sort direction - ascending/descending
     */

    bool sortAscending() const
    {
        return m_sortAscending;
    }

    /**
     * Sets the sort direction - ascending/descending
     * @param ascending bool, sort direction - ascending/descending
     * @param sortNow bool, true if you want to sort immediatedly
     */
    void sortAscending(bool ascending, bool sortNow);

    /**
     * Returns summary height of all the rows
     */

    int32_t fullHeight() const
    {
        return(int32_t) m_fullHeight;
    }
};
/**
 * @}
 */
}
#endif
