/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/Printer.h>
#include <sptk5/gui/CColumn.h>

using namespace std;
using namespace sptk;

CColumn::CColumn(const string& cname, VariantDataType type, int32_t cwidth, bool cvisible)
    : m_name(cname)
    , m_type(type)
    , m_visible(cvisible)
{
    if (cwidth < 0)
    {
        m_autoWidth = true;
        m_width = 0;
    }
    else
    {
        m_autoWidth = false;
        m_width = (uint32_t) cwidth;
    }
}

void CColumn::load(const xdoc::SNode& node)
{
    m_name = node->attributes().get("caption");
    m_type = (VariantDataType) node->attributes().get("type").toInt();
    m_width = node->attributes().get("width").toInt();
    m_visible = node->attributes().get("visible") == "true";
    m_autoWidth = node->attributes().get("auto_width") == "true";
}

void CColumn::save(const xdoc::SNode& node) const
{
    node->clear();
    node->setName("column");
    node->attributes().set("caption", m_name);
    node->attributes().set("type", to_string((int) m_type));
    node->attributes().set("width", to_string(m_width));
    node->attributes().set("visible", m_visible ? "true" : "false");
    node->attributes().set("auto_width", m_autoWidth ? "true" : "false");
}

int CColumnList::indexOf(const char* colname) const
{
    size_t cnt = size();
    for (size_t i = 0; i < cnt; i++)
    {
        const CColumn& column = operator[](i);
        if (column.name() == colname)
        {
            return int(i);
        }
    }
    return -1;
}

void CColumnList::load(const xdoc::SNode& node)
{
    resize(node->nodes().size());
    for (const auto& columnNode: node->nodes())
    {
        try
        {
            unsigned columnIndex = columnNode->attributes().get("index").toInt();
            if (columnIndex >= size())
            {
                continue;
            }
            CColumn& column = (*this)[columnIndex];
            column.load(columnNode);
        }
        catch (const Exception& e)
        {
            CERR(e.what());
        }
    }
}

void CColumnList::save(const xdoc::SNode& node) const
{
    node->clear();
    node->setName("columns");
    size_t counter = size();
    for (size_t i = 0; i < counter; i++)
    {
        try
        {
            const CColumn& column = (*this)[i];
            const auto&    columnNode = node->pushNode("column");
            column.save(columnNode);
            columnNode->attributes().set("index", to_string(i));
        }
        catch (const Exception& e)
        {
            CERR(e.what());
        }
    }
}
