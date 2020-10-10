/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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
#include <sptk5/Printer.h>
#include <sptk5/gui/CColumn.h>

using namespace std;
using namespace sptk;

CColumn::CColumn(const string& cname, VariantType type, int32_t cwidth, bool cvisible)
: m_name(cname), m_type(type), m_visible(cvisible)

{
    if (cwidth < 0) {
        m_autoWidth = true;
        m_width = 0;
    } else {
        m_autoWidth = false;
        m_width = (uint32_t) cwidth;
    }
}

void CColumn::load(const xml::Node& node)
{
    m_name = (String) node.getAttribute("caption");
    m_type = (VariantType) (int) node.getAttribute("type");
    m_width = (int) node.getAttribute("width");
    m_visible = (bool) node.getAttribute("visible");
    m_autoWidth = (bool) node.getAttribute("auto_width");
}

void CColumn::save(xml::Node& node) const
{
    node.clear();
    node.name("column");
    node.setAttribute("caption", m_name);
    node.setAttribute("type", (int) m_type);
    node.setAttribute("width", (int) m_width);
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
            unsigned columnIndex = (int) columnNode.getAttribute("index");
            if (columnIndex >= size())
                continue;
            CColumn& column = (*this)[columnIndex];
            column.load(columnNode);
        } catch (const Exception& e) {
            CERR(e.what() << endl)
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
            columnNode.setAttribute("index", (int) i);
        } catch (const Exception& e) {
            CERR(e.what() << endl)
        }
    }
}
