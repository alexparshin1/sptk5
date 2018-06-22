/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CListViewSelection.cpp - description                   ║
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
#include <sptk5/gui/CListViewSelection.h>

using namespace sptk;

void CSelection::select(CPackedStrings* row)
{
    if (row) {
        row->flags |= CLV_SELECTED;
        m_selectedRows.push_back(row);
    }
}

void CSelection::deselect(CPackedStrings* row)
{
    if (row) {
        row->flags &= ~CLV_SELECTED;
        remove(row);
    }
}

void CSelection::deselectAll()
{
    size_t cnt = m_selectedRows.size();
    for (size_t i = 0; i < cnt; i++) {
        auto row = (CPackedStrings*) m_selectedRows[i];
        row->flags &= ~CLV_SELECTED;
    }
    m_selectedRows.clear();
}

void CSelection::remove(CPackedStrings* row)
{
    auto itor = std::find(m_selectedRows.begin(), m_selectedRows.end(), row);
    if (itor != m_selectedRows.end())
        m_selectedRows.erase(itor);
}

void CSelection::clear()
{
    m_selectedRows.clear();
}

CPackedStrings* CSelection::findKey(int keyValue) const
{
    size_t cnt = m_selectedRows.size();
    for (size_t i = 0; i < cnt; i++) {
        auto row = (CPackedStrings*) m_selectedRows[i];
        if (row->argument() == keyValue)
            return row;
    }
    return nullptr;
}

CPackedStrings* CSelection::findCaption(const String& caption) const
{
    size_t cnt = m_selectedRows.size();
    for (size_t i = 0; i < cnt; i++) {
        auto row = (CPackedStrings*) m_selectedRows[i];
        if ((*row)[0] == caption)
            return row;
    }
    return nullptr;
}
