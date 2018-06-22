/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CListViewSelection.h - description                     ║
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

#ifndef __CLISTVIEWSELECTION_H__
#define __CLISTVIEWSELECTION_H__

#include <sptk5/PackedStrings.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

#define CLV_SELECTED       1    ///< List view item is selected
#define CLV_CHECKED        2    ///< List view item is checked
#define CLV_NOTDISPLAYED   4    ///< List view item is hidden

/**
 * @brief List view & friend items selection
 *
 * Allows CListView to work with one or few selected rows
 * without scanning through all the rows in list view for selected rows.
 */

class SP_EXPORT CSelection
{
    typedef CPackedStrings * PPackedStrings;
    typedef std::vector<PPackedStrings> CPSVector;

    /**
     * The list of the rows
     */
    CPSVector m_selectedRows;


public:

    /**
     * Default constructor
     */

    CSelection()
    {
    }

    /**
     * Destructor
     */

    ~CSelection()
    {
        clear();
    }

    /**
     * Selects a row and adds it to the selection list
     */
    void select(CPackedStrings *row);

    /**
     * Deselects a row and removes it from the selection list
     */
    void deselect(CPackedStrings *row);

    /**
     * Removes the row from the selection list
     */
    void remove
    (CPackedStrings *row);

    /**
     * Deselects all the rows in the selection list and empties the list
     */
    void deselectAll();

    /**
     * Removes all rows from the selection list
     */
    void clear();

    /**
     * Returns the number of selected rows
     */

    uint32_t size() const
    {
        return(uint32_t) m_selectedRows.size();
    }

    /**
     * Element access to the selection, const version only
     * @param index int, index in the selection list
     */
    CPackedStrings& operator[] (int index) const
    {
        return *m_selectedRows[size_t(index)];
    }

    /**
     * Finds an item with a particular key value. Returns NULL if not found.
     * @param keyValue int, key value
     */
    CPackedStrings *findKey(int keyValue) const;

    /**
     * Finds an item with a particular caption. Returns NULL if not found.
     * @param caption CString, item caption (a string in the first column)
     */
    CPackedStrings *findCaption(const String& caption) const;
};
/**
 * @}
 */
}
#endif
