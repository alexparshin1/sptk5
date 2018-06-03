/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CColorSchema.cpp - description                         ║
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

#include <sptk5/gui/CColorSchema.h>

using namespace std;
using namespace sptk;

Fl_Color CColorSchema::m_colorMapIndex[] =
{
  FL_FOREGROUND_COLOR, FL_BACKGROUND2_COLOR, FL_INACTIVE_COLOR, FL_SELECTION_COLOR,             /// Window colors
  FL_GRAY0, FL_DARK3, FL_DARK2, FL_DARK1, FL_BACKGROUND_COLOR, FL_LIGHT1, FL_LIGHT2, FL_LIGHT3, /// Box colors
  FL_WHITE                                                                                      /// List termination
};

void CColorSchema::store()
{
    for (unsigned i = 0; m_colorMapIndex[i] != FL_WHITE; i++ ) {
        Fl_Color color = m_colorMapIndex[i];
        m_colorMap[color] = Fl::get_color(color);
    }
}

void CColorSchema::restore()
{
    for (auto itor: m_colorMap)
        Fl::set_color(itor.first, itor.second);
}

