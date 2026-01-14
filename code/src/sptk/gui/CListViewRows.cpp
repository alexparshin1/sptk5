/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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
    m_sortColumnType = VariantDataType::VAR_STRING;
    m_fullHeight = 0;
}

CListViewRows::~CListViewRows()
{
    clear();
}

void CListViewRows::truncate(unsigned cnt)
{
    if (const auto rowCount = m_rows.size();
        cnt < rowCount)
    {
        for (size_t i = cnt; i < rowCount; i++)
        {
            const auto* row = m_rows[i];
            m_fullHeight -= row->height;
            delete row;
        }
        m_rows.resize(cnt);
    }
}

unsigned CListViewRows::add(CPackedStrings* row)
{
    const int lineNumber = size();
    m_rows.push_back(row);
    m_fullHeight += row->height;
    return static_cast<unsigned>(lineNumber);
}

unsigned CListViewRows::insert(unsigned position, CPackedStrings* ss)
{
    m_rows.insert(m_rows.begin() + position, ss);
    m_fullHeight += ss->height;
    return position;
}

unsigned CListViewRows::update(unsigned index, CPackedStrings* ss)
{
    const auto* s = m_rows[index];
    const int   oldh = s->height;
    delete s;
    m_fullHeight += ss->height - oldh;
    m_rows[index] = ss;
    return index;
}

void CListViewRows::clear()
{
    for (const auto* packedStrings: m_rows)
    {
        delete packedStrings;
    }
    m_rows.clear();
    m_fullHeight = 0;
}

void CListViewRows::remove(unsigned index)
{
    if (index < m_rows.size())
    {
        const auto* row = m_rows[index];
        m_fullHeight -= row->height;
        delete row;
        m_rows.erase(m_rows.begin() + index);
    }
}

int CListViewRows::currentSortColumn;

bool CListViewRows::compare_strings(const PPackedStrings& a, const PPackedStrings& b)
{
    return (*a)[currentSortColumn] < (*b)[currentSortColumn];
}

bool CListViewRows::compare_integers(const PPackedStrings& a, const PPackedStrings& b)
{
    const auto i1 = string2int((*a)[currentSortColumn]);
    const auto i2 = string2int((*b)[currentSortColumn]);
    return i1 < i2;
}

bool CListViewRows::compare_floats(const PPackedStrings& a, const PPackedStrings& b)
{
    const auto d1 = string2double((*a)[currentSortColumn]);
    const auto d2 = string2double((*b)[currentSortColumn]);
    return d1 < d2;
}

bool CListViewRows::compare_dates(const PPackedStrings& a, const PPackedStrings& b)
{
    const DateTime d1((*a)[currentSortColumn].c_str());
    const DateTime d2((*b)[currentSortColumn].c_str());
    return d1 < d2;
}

bool CListViewRows::compare_datetimes(const PPackedStrings& a, const PPackedStrings& b)
{
    const DateTime d1((*a)[currentSortColumn].c_str());
    const DateTime d2((*b)[currentSortColumn].c_str());
    return d1 < d2;
}

void CListViewRows::sort()
{
    fl_cursor(FL_CURSOR_WAIT);
    Fl::check();

    if (const auto m_size = m_rows.size();
        m_sortColumn >= 0 && m_size > 1)
    {
        currentSortColumn = m_sortColumn;
        switch (m_sortColumnType)
        {
            using enum VariantDataType;
            case VAR_BOOL:
            case VAR_INT:
                ranges::sort(m_rows, compare_integers);
                break;
            case VAR_FLOAT:
                ranges::sort(m_rows, compare_floats);
                break;
            case VAR_DATE:
                ranges::sort(m_rows, compare_dates);
                break;
            case VAR_DATE_TIME:
                ranges::sort(m_rows, compare_datetimes);
                break;
            default:
                ranges::sort(m_rows, compare_strings);
                break;
        }
        if (!m_sortAscending)
        {
            // reversing sort order for the descending sort
            const auto cnt = m_rows.size();
            const auto mid = cnt / 2;
            auto       j = cnt - 1;
            for (size_t i = 0; i < mid; i++, j--)
            {
                CPackedStrings* item = m_rows[i];
                m_rows[i] = m_rows[j];
                m_rows[j] = item;
            }
        }
    }
    fl_cursor(FL_CURSOR_DEFAULT);
    Fl::check();
}

void CListViewRows::sortColumn(int column, VariantDataType columnType, bool sortNow)
{
    m_sortColumn = column;
    m_sortColumnType = columnType;
    if (sortNow)
    {
        sort();
    }
}

void CListViewRows::sortAscending(bool ascending, bool sortNow)
{
    m_sortAscending = ascending;
    if (sortNow)
    {
        sort();
    }
}

int CListViewRows::indexOf(const CPackedStrings* ss) const
{
    const auto itor = ranges::find(m_rows, ss);
    if (itor == m_rows.end())
    {
        return -1;
    }
    return static_cast<int>(distance(m_rows.begin(), itor));
}
