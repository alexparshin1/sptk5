/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CDBListView.h - description                            ║
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

#ifndef __DBLISTVIEW_H__
#define __DBLISTVIEW_H__

#include <sptk5/gui/CListView.h>
#include <sptk5/db/Query.h>
#include <sptk5/DateTime.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief List view widget with extended database support
 */
class SP_EXPORT CDBListView : public CListView
{
protected:
    /**
     * Full refersh Query
     */
    Query      m_fullRefreshQuery;

    /**
     * Fast refersh Query
     */
    Query      m_fastRefreshQuery;

    /**
     * Record count Query
     */
    Query      m_recordCountQuery;

    /**
     * Key field name
     */
    String      m_keyField;

    /**
     * True if fast refresh is defined properly
     */
    bool        m_fastRefreshEnabed;

    /**
     * Last refresh date and time
     */
    DateTime   m_lastRefresh;

    /**
     * Record fetch limit
     */
    uint32_t    m_maxRecords;

    /**
     * Is the records limit enabled?
     */
    bool        m_recordsLimited;

public:

    /**
     * Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CDBListView(const char *label=0,int layoutSize=20,CLayoutAlign layoutAlign=SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CDBListView(int x, int y, int w, int h, const char *label=0);
#endif

    /**
     * Destructor
     */
    ~CDBListView();

    /**
     * Sets the database connection
     */
    void database(PoolDatabaseConnection *db);

    /**
     * Returns the database connection
     */
    PoolDatabaseConnection *database() const;

    /**
     * Sets the SQL queries. Both full and fast refresh queries should return the same set of fields.
     * The record count query should return only number of record.
     * @param sql               The SQL query for full data refresh
     * @param recordCountSql    The SQL query for obtaining record count
     * @param fastRefreshSQL    The SQL query for retrieving only records changed since last refresh
     */
    void sql(const String& sql, const String& recordCountSql = "", const String& fastRefreshSQL = "");

    /**
     * Return the SQL query text
     */
    String sql();

    /**
     * Sets the query parameter
     * @param paramName         The parameter Name
     * @param refreshKind       The query it belongs to (full or fast refresh)
     */
    QueryParameter& param(const char *paramName,CRefreshKind refreshKind = LV_REFRESH_FULL);

    /**
     * Defines the key field name. This field name should be a part of the SQL query
     * and contain the unique integer values so the row could be identified by that value.
     * @param fieldName std::string, the name of the field
     */
    void keyField(const String& fieldName);

    /**
     * Returns key field name
     */
    std::string keyField() const
    {
        return m_keyField;
    }

    /**
     * Fast setup of the database connection
     * @param db CDatabase *, the database connection
     * @param sql std::string,  the full refresh SQL query text
     * @param keyField std::string, the name of the key field
     */
    void setup(PoolDatabaseConnection* db, const String& sql, const String& keyField);

    /**
     * Refreshes the data with full or fast method
     * @param refreshKind CRefreshKind, the type of refresh
     */
    void refreshData(CRefreshKind refreshKind = LV_REFRESH_FULL);

    /**
     * Returns the date and time of the last data refresh
     */
    DateTime lastRefresh() const {
        return m_lastRefresh;
    }

    /**
     * Sets the maximum record number to fetch from the database
     */
    void maxRecords(uint32_t mr)
    {
        m_maxRecords = mr;
    }

    /**
     * Returns the maximum record number to fetch from the database
     */
    uint32_t maxRecords() const
    {
        return m_maxRecords;
    }

    /**
     * Returns the records limited flag. The flag is set if the query
     * returned more records than records limit.
     */
    bool recordsLimited() const
    {
        return m_recordsLimited;
    }

    /**
     * @brief Creates a widget based on the XML node information
     * @param node xml::Node*, an XML node with widget information
     */
    static CLayoutClient* creator(xml::Node* node);
};
/**
 * @}
 */
}

#endif
