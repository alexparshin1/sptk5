/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CColumn.cpp - description                              ║
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

#include <sptk5/gui/CColumn.h>

using namespace std;
using namespace sptk;

CColumn::CColumn(const string& cname, VariantType type, int32_t cwidth, bool cvisible)
{
    m_name = cname;
    m_type = type;
    m_visible = cvisible;
    if (cwidth < 0) {
        m_autoWidth = true;
        m_width = 0;
    } else {
        m_autoWidth = false;
        m_width = (uint32_t) cwidth;
    }
}

CColumn::CColumn(const CColumn& col)
{
    m_name = col.m_name;
    m_type = col.m_type;
    m_width = col.m_width;
    m_visible = col.m_visible;
    m_autoWidth = col.m_autoWidth;
}

void CColumn::load(const xml::Node& node)
{
    m_name = (string) node.getAttribute("caption");
    m_type = (VariantType) (unsigned) node.getAttribute("type");
    m_width = node.getAttribute("width");
    m_visible = node.getAttribute("visible");
    m_autoWidth = node.getAttribute("auto_width");
}

void CColumn::save(xml::Node& node) const
{
    node.clear();
    node.name("column");
    node.setAttribute("caption", m_name);
    node.setAttribute("type", m_type);
    node.setAttribute("width", m_width);
    node.setAttribute("visible", m_visible);
    node.setAttribute("auto_width", m_autoWidth);
}

int CColumnList::indexOf(const char *colname) const
{
    size_t cnt = size();
    for (size_t i = 0; i < cnt; i++) {
        const CColumn& column = operator[](i);
        if (column.name() == colname)
            return int(i);
    }
    return -1;
}

void CColumnList::load(const xml::Node& node)
{
    auto itor = node.begin();
    auto iend = node.end();
    resize(node.size());
    for (; itor != iend; ++itor) {
        try {
            xml::Node& columnNode = *(*itor);
            unsigned columnIndex = columnNode.getAttribute("index");
            if (columnIndex >= size())
                continue;
            CColumn& column = (*this)[columnIndex];
            column.load(columnNode);
        } catch (...) {
        }
    }
}

void CColumnList::save(xml::Node& node) const
{
    node.clear();
    node.name("columns");
    size_t counter = size();
    for (size_t i = 0; i < counter; i++) {
        try {
            const CColumn& column = (*this)[i];
            xml::Node& columnNode = *(new xml::Element(node, "column"));
            column.save(columnNode);
            columnNode.setAttribute("index", i);
        } catch (...) {
        }
    }
}
