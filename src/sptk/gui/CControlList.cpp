/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CControlList.h  -  description
                             -------------------
    begin                : 12 June 2003
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <FL/Fl_Group.H>

#include <string.h>
#include <stdlib.h>
#include <sptk5/CException.h>
#include <sptk5/gui/CControlList.h>

using namespace sptk;

void CControlList::scanControls(const Fl_Group *group)
{
    unsigned cnt = group->children();
    for (unsigned i = 0; i < cnt; i++) {
        Fl_Widget *widget = group->child(i);

        // The try {} catch() {} is only required for MSVC++
        CControl *control = 0L;
        try {
            control = dynamic_cast<CControl *>(widget);
        } catch (...) {
        }

        if (control) {
            if (control->fieldName().length())
                add(control);
            continue;
        }

        Fl_Group *g = 0L;

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
    const_iterator itor = list.begin();
    for (; itor != list.end(); itor++)
        add(itor->second);
}

void CControlList::remove(const CControlList& l)
{
    const_iterator itor = l.begin();
    for (; itor != l.end(); itor++)
        remove(itor->second);
}

void CControlList::reset()
{
    iterator itor = begin();
    for (; itor != end(); itor++)
        itor->second->reset();
}
