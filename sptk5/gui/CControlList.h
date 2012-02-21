/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          ccontrollistcslists.h  -  description
                             -------------------
    begin                : 12 June 2003
    copyright            : (C) 2000-2012 by Alexey Parshin. All rights reserved.
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

#ifndef __CCONTROLLIST_H__
#define __CCONTROLLIST_H__

#include <sptk5/istring.h>
#include <sptk5/gui/CControl.h>
#include <map>

namespace sptk {

/// @addtogroup gui GUI Classes
/// @{

/// String to Control map.
/// Uses case-insensitive strings and pointers to CControl.
/// Strings are control field names.
typedef std::map<istring,CControl *> CStringControlMap;

/// List of CControl object pointers in Fl_Group
class CControlList : public CStringControlMap {
protected:
    /// Scan group to find all CControl objects inside,
    /// including children groups
    void scanControls(const Fl_Group *group);
public:
    /// Constructor
    CControlList() {}

    /// Adds a CControl pointer into the list
    void add
        (CControl *control);

    /// Adds a list of CControl pointers into the list
    void add
        (const CControlList& l);

    /// Adds a list of CControl pointers from the group into the list
    void add
        (const Fl_Group& g);

    /// Removes CControl pointer from the list
    void remove
        (CControl *control) {
        erase(control->fieldName());
    }

    /// Removes a list of CControl pointers from the list
    void remove
        (const CControlList& l);

    /// Returns true if the control for the same field name exists
    bool contains(CControl *control) const {
        if (!control)
            return false;
        return find(control->fieldName()) != end();
    }

    /// Assignment operation
    CControlList& operator =  (const Fl_Group& g)     {
        clear();
        add
            (g);
        return *this;
    }
    /// Assignment operation
    CControlList& operator =  (const CControlList& l) {
        clear();
        add
            (l);
        return *this;
    }

    /// Addition operation
    CControlList& operator << (CControl *c)           {
        add
            (c);
        return *this;
    }

    /// Addition operation
    CControlList& operator << (const Fl_Group& g)     {
        add
            (g);
        return *this;
    }

    /// Addition operation
    CControlList& operator << (const CControlList& l) {
        add
            (l);
        return *this;
    }

    /// Sends reset() signal to all the widgets in the list
    void reset();
};
/// @}
}
#endif
