/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/gui/CFont.h>

using namespace std;
using namespace sptk;

string CFont::id() const
{
    return m_name + "|" + int2string(m_index) + "|" + int2string(m_size) + "|" + int2string(m_color);
}

void CFontsMap::clear()
{
    for (const auto& itor: *this)
    {
        delete itor.second;
    }
    map<String, CFont*, less<>>::clear();
}

void CFontsVector::clear()
{
    m_items.clear();
    m_index.clear();
}

void CFontsVector::push_back(CFont* font)
{
    m_items.push_back(font);
    m_index[font->name()] = font;
}

CFont* CFontsVector::find(const String& fontName) const
{
    auto itor = m_index.find(fontName);
    if (itor == m_index.end())
    {
        return nullptr;
    }
    return itor->second;
}
