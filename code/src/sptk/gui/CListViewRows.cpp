/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CListViewRows.cpp - description                        ║
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

#include <FL/Fl.H>
#include <FL/fl_draw.H>

#include <sptk5/gui/CListViewRows.h>

using namespace std;
using namespace sptk;

CListViewRows::CListViewRows()
{
    m_sortColumn = -1;
    m_sortAscending = true;
    m_sortColumnType = VAR_STRING;
    m_fullHeight = 0;
}

CListViewRows::~CListViewRows()
{
    clear();
}

void CListViewRows::truncate(unsigned cnt)
{
    size_t rowCount = m_rows.size();
    if (cnt < rowCount) {
        for (size_t i = cnt; i < rowCount; i++) {
            auto row = (CPackedStrings*) m_rows[i];
            m_fullHeight -= row->height;
            delete row;
        }
        m_rows.resize(cnt);
    }
}

unsigned CListViewRows::add(CPackedStrings* ss)
{
    int lineNumber = size();
    m_rows.push_back(ss);
    m_fullHeight += ss->height;
    return (unsigned) lineNumber;
}

unsigned CListViewRows::insert(unsigned position, CPackedStrings* ss)
{
    m_rows.insert(m_rows.begin() + position, ss);
    m_fullHeight += ss->height;
    return position;
}

unsigned CListViewRows::update(unsigned index, CPackedStrings* ss)
{
    auto s = (CPackedStrings*) m_rows[index];
    int oldh = s->height;
    delete s;
    m_fullHeight += ss->height - oldh;
    m_rows[index] = ss;
    return index;
}

void CListViewRows::clear()
{
    for (auto packedStrings: m_rows)
        delete packedStrings;
    m_rows.clear();
    m_fullHeight = 0;
}

void CListViewRows::remove(unsigned index)
{
    if (index < m_rows.size()) {
        auto row = (CPackedStrings*) m_rows[index];
        m_fullHeight -= row->height;
        delete row;
        m_rows.erase(m_rows.begin() + index);
    }
}

int CListViewRows::currentSortColumn;

bool CListViewRows::compare_strings(const PPackedStrings& a, const PPackedStrings& b)
{
    return strcmp((*a)[currentSortColumn], (*b)[currentSortColumn]) < 0;
}

bool CListViewRows::compare_integers(const PPackedStrings& a, const PPackedStrings& b)
{
    int i1 = string2int((*a)[currentSortColumn]);
    int i2 = string2int((*b)[currentSortColumn]);
    return i1 < i2;
}

bool CListViewRows::compare_floats(const PPackedStrings& a, const PPackedStrings& b)
{
    double d1 = string2double((*a)[currentSortColumn]);
    double d2 = string2double((*b)[currentSortColumn]);
    return d1 < d2;
}

bool CListViewRows::compare_dates(const PPackedStrings& a, const PPackedStrings& b)
{
    DateTime d1((*a)[currentSortColumn]);
    DateTime d2((*b)[currentSortColumn]);
    return d1 < d2;
}

bool CListViewRows::compare_datetimes(const PPackedStrings& a, const PPackedStrings& b)
{
    DateTime d1((*a)[currentSortColumn]);
    DateTime d2((*b)[currentSortColumn]);
    return d1 < d2;
}

void CListViewRows::sort()
{
    fl_cursor(FL_CURSOR_WAIT);
    Fl::check();

    size_t m_size = m_rows.size();
    if (m_sortColumn >= 0 && m_size > 1) {
        CListViewRows::currentSortColumn = m_sortColumn;
        switch (m_sortColumnType) {
            case VAR_BOOL:
            case VAR_INT:
                std::sort(m_rows.begin(), m_rows.end(), compare_integers);
                break;
            case VAR_FLOAT:
                std::sort(m_rows.begin(), m_rows.end(), compare_floats);
                break;
            case VAR_DATE:
                std::sort(m_rows.begin(), m_rows.end(), compare_dates);
                break;
            case VAR_DATE_TIME:
                std::sort(m_rows.begin(), m_rows.end(), compare_datetimes);
                break;
            default:
                std::sort(m_rows.begin(), m_rows.end(), compare_strings);
                break;
        }
        if (!m_sortAscending) {
            // reversing sort order for the descending sort
            size_t cnt = m_rows.size();
            size_t mid = cnt / 2;
            size_t j = cnt - 1;
            for (size_t i = 0; i < mid; i++, j--) {
                CPackedStrings* item = m_rows[i];
                m_rows[i] = m_rows[j];
                m_rows[j] = item;
            }
        }
    }
    fl_cursor(FL_CURSOR_DEFAULT);
    Fl::check();
}

void CListViewRows::sortColumn(int column, VariantType columnType, bool sortNow)
{
    m_sortColumn = column;
    m_sortColumnType = columnType;
    if (sortNow) sort();
}

void CListViewRows::sortAscending(bool ascending, bool sortNow)
{
    m_sortAscending = ascending;
    if (sortNow) sort();
}

int CListViewRows::indexOf(CPackedStrings* ss) const
{
    auto itor = find(m_rows.begin(), m_rows.end(), ss);
    if (itor == m_rows.end()) return -1;
    return (int) distance(m_rows.begin(), itor);
}
