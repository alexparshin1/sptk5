/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CControlList.cpp - description                         ║
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

#include <FL/Fl_Group.H>
#include <sptk5/gui/CControlList.h>

using namespace sptk;

void CControlList::scanControls(const Fl_Group *group)
{
    auto cnt = (unsigned) group->children();
    for (unsigned i = 0; i < cnt; i++) {
        Fl_Widget *widget = group->child(i);

        // The try {} catch() {} is only required for MSVC++
        CControl *control = nullptr;
        try {
            control = dynamic_cast<CControl *>(widget);
        } catch (...) {
        }

        if (control) {
            if (control->fieldName().length())
                add(control);
            continue;
        }

        Fl_Group *g = nullptr;

        try {
            g = dynamic_cast<Fl_Group *>(widget);
        } catch (...) {
        }

        if (g)
            scanControls(g);
    }
}

void CControlList::add(CControl *control)
{
    (*this)[control->fieldName()] = control;
}

void CControlList::add(const Fl_Group& group)
{
    scanControls(&group);
}

void CControlList::add(const CControlList& list)
{
    auto itor = list.begin();
    for (; itor != list.end(); ++itor)
        add(itor->second);
}

void CControlList::remove(const CControlList& l)
{
    for (auto itor: l)
        remove(itor.second);
}

void CControlList::reset()
{
    for (auto itor: *this)
        itor.second->reset();
}
