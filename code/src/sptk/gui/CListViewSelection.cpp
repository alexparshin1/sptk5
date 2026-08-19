/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/gui/CListViewSelection.h>

using namespace sptk;

void CSelection::select(CPackedStrings* row)
{
    if (row)
    {
        row->flags |= CLV_SELECTED;
        m_selectedRows.push_back(row);
    }
}

void CSelection::deselect(CPackedStrings* row)
{
    if (row)
    {
        row->flags &= ~CLV_SELECTED;
        remove(row);
    }
}

void CSelection::deselectAll()
{
    const size_t cnt = m_selectedRows.size();
    for (size_t i = 0; i < cnt; i++)
    {
        auto* row = (CPackedStrings*) m_selectedRows[i];
        row->flags &= ~CLV_SELECTED;
    }
    m_selectedRows.clear();
}

void CSelection::remove(CPackedStrings* row)
{
    const auto itor = std::find(m_selectedRows.begin(), m_selectedRows.end(), row);
    if (itor != m_selectedRows.end())
    {
        m_selectedRows.erase(itor);
    }
}

void CSelection::clear()
{
    m_selectedRows.clear();
}

CPackedStrings* CSelection::findKey(int keyValue) const
{
    const size_t cnt = m_selectedRows.size();
    for (size_t i = 0; i < cnt; i++)
    {
        auto* row = (CPackedStrings*) m_selectedRows[i];
        if (row->argument() == keyValue)
        {
            return row;
        }
    }
    return nullptr;
}

CPackedStrings* CSelection::findCaption(const String& caption) const
{
    const size_t cnt = m_selectedRows.size();
    for (size_t i = 0; i < cnt; i++)
    {
        auto* row = (CPackedStrings*) m_selectedRows[i];
        if ((*row)[0] == caption)
        {
            return row;
        }
    }
    return nullptr;
}
