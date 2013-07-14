/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDBListview.h  -  description
                             -------------------
    begin                : Fri Mar 31 2000
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#ifndef __DBLISTVIEW_H__
#define __DBLISTVIEW_H__

#include <sptk5/gui/CListView.h>
#include <sptk5/db/CQuery.h>
#include <sptk5/CDateTime.h>

namespace sptk {

/// @addtogroup gui GUI Classes
/// @{

/// @brief List view widget with extended database support
class SP_EXPORT CDBListView : public CListView
{
protected:
    CQuery      m_fullRefreshQuery;  ///< Full refersh Query
    CQuery      m_fastRefreshQuery;  ///< Fast refersh Query
    CQuery      m_recordCountQuery;  ///< Record count Query
    std::string m_keyField;          ///< Key field name
    bool        m_fastRefreshEnabed; ///< True if fast refresh is defined properly
    CDateTime   m_lastRefresh;       ///< Last refresh date and time
    uint32_t    m_maxRecords;        ///< Record fetch limit
    bool        m_recordsLimited;    ///< Is the records limit enabled?
public:

    /// Constructor in SPTK style
    /// @param label const char *, label
    /// @param layoutSize int, widget align in layout
    /// @param layoutAlign CLayoutAlign, widget align in layout
    CDBListView(const char *label=0,int layoutSize=20,CLayoutAlign layoutAlign=SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /// Constructor in FLTK style
    /// @param x int, x-position
    /// @param y int, y-position
    /// @param w int, width
    /// @param h int, height
    /// @param label, const char * label
    CDBListView(int x, int y, int w, int h, const char *label=0);
#endif

    /// Destructor
    ~CDBListView();

    /// Sets the database connection
    void database(CDatabaseConnection *db);

    /// Returns the database connection
    CDatabaseConnection *database() const;

    /// Sets the SQL queries. Both full and fast refresh queries should return the same set of fields.
    /// The record count query should return only number of record.
    /// @param sql std::string, the SQL query for full data refresh
    /// @param recordCountSql std::string, the SQL query for obtaining record count
    /// @param fastRefreshSQL std::string, the SQL query for retrieving only records changed since last refresh
    void sql(std::string sql,std::string recordCountSql="",std::string fastRefreshSQL="");

    /// Return the SQL query text
    std::string sql();

    /// Sets the query parameter
    /// @param paramName const char *, the parameter Name
    /// @param refreshKind CRefreshKind, the query it belongs to (full or fast refresh)
    CParam& param(const char *paramName,CRefreshKind refreshKind = LV_REFRESH_FULL);

    /// Defines the key field name. This field name should be a part of the SQL query
    /// and contain the unique integer values so the row could be identified by that value.
    /// @param fieldName std::string, the name of the field
    void keyField(std::string fieldName);

    /// Returns key field name
    std::string keyField() const
    {
        return m_keyField;
    }

    /// Fast setup of the database connection
    /// @param db CDatabase *, the database connection
    /// @param sql std::string,  the full refresh SQL query text
    /// @param keyField std::string, the name of the key field
    void setup(CDatabaseConnection* db,std::string sql,std::string keyField);

    /// Refreshes the data with full or fast method
    /// @param refreshKind CRefreshKind, the type of refresh
    void refreshData(CRefreshKind refreshKind = LV_REFRESH_FULL);

    /// Returns the date and time of the last data refresh
    CDateTime lastRefresh() const {
        return m_lastRefresh;
    }

    /// Sets the maximum record number to fetch from the database
    void maxRecords(uint32_t mr)
    {
        m_maxRecords = mr;
    }

    /// Returns the maximum record number to fetch from the database
    uint32_t maxRecords() const
    {
        return m_maxRecords;
    }

    /// Returns the records limited flag. The flag is set if the query
    /// returned more records than records limit.
    bool recordsLimited() const
    {
        return m_recordsLimited;
    }

    /// @brief Creates a widget based on the XML node information
    /// @param node CXmlNode*, an XML node with widget information
    static CLayoutClient* creator(CXmlNode* node);
};
/// @}
}

#endif
