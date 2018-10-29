/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CDBListView.cpp - description                          ║
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

#include <sptk5/sptk.h>

#include <sptk5/FieldList.h>
#include <sptk5/gui/CDBListView.h>
#include <sptk5/gui/CMessageDialog.h>

#include <FL/fl_draw.H>

using namespace std;
using namespace sptk;

CDBListView::CDBListView(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
    : CListView(label, layoutSize, layoutAlignment)
{
    m_fastRefreshEnabed = false;
    m_maxRecords = 0;
    m_recordsLimited = false;
}

#ifdef __COMPATIBILITY_MODE__
CDBListView::CDBListView(int x, int y, int w, int h, const char *label)
: CListView(x, y, w, h, label) {
   m_fastRefreshEnabed = false;
   m_maxRecords = 0;
   m_recordsLimited = false;
}
#endif

CDBListView::~CDBListView()
{
    m_fastRefreshQuery.close();
    m_fullRefreshQuery.close();
}

CLayoutClient* CDBListView::creator(xml::Node* node)
{
    auto widget = new CDBListView("", 10, SP_ALIGN_TOP);
    widget->load(node, LXM_LAYOUTDATA);
    return widget;
}

void CDBListView::database(PoolDatabaseConnection* db)
{
    if (m_dataMode == LV_DATA_UNDEFINED)
        m_dataMode = LV_DATA_KEY;
    m_fastRefreshQuery.connect(db);
    m_fullRefreshQuery.connect(db);
    m_recordCountQuery.connect(db);
}

PoolDatabaseConnection* CDBListView::database() const
{
    return m_fullRefreshQuery.database();
}

void CDBListView::sql(const String& _sql, const String& _recordCountSql, const String& _fastRefreshSQL)
{
    m_fullRefreshQuery.sql(_sql);
    m_fastRefreshQuery.sql(_fastRefreshSQL);
    m_recordCountQuery.sql(_recordCountSql);
    m_fastRefreshEnabed = _fastRefreshSQL.length() > 0;
}

String CDBListView::sql()
{
    return m_fullRefreshQuery.sql();
}

QueryParameter& CDBListView::param(const char* paramName, CRefreshKind refreshKind)
{
    if (m_fastRefreshEnabed && refreshKind == LV_REFRESH_FAST)
        return m_fastRefreshQuery.param(paramName);
    return m_fullRefreshQuery.param(paramName);
}

void CDBListView::setup(PoolDatabaseConnection* db, const String& _sql, const String& _keyField)
{
    database(db);
    sql(_sql);
    keyField(_keyField);
}

void CDBListView::keyField(const String& keyField)
{
    m_keyField = keyField;
}

void CDBListView::refreshData(CRefreshKind refreshKind)
{
    m_lastRefresh = DateTime::Now();

    fl_cursor(FL_CURSOR_WAIT);
    Fl::check();

    unsigned recordsEstimated = 0;
    if (!m_recordCountQuery.sql().empty()) {
        m_recordCountQuery.open();
        recordsEstimated = (unsigned) m_recordCountQuery[uint32_t(0)].asInteger();
        m_recordCountQuery.close();
    }

    Query* query = &m_fullRefreshQuery;
    if (m_fastRefreshEnabed && refreshKind == LV_REFRESH_FAST)
        query = &m_fastRefreshQuery;
    else refreshKind = LV_REFRESH_FULL;

    fill(*query, m_keyField, m_maxRecords, recordsEstimated, refreshKind);
}
