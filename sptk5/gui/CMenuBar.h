/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CMenuBar.h - description                               ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
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

#ifndef __CMENUBAR_H__
#define __CMENUBAR_H__

#include <FL/Fl_Menu_Bar.H>
#include <sptk5/gui/CLayoutClient.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Menu bar
 *
 * Extended version of FLTK's standard Fl_Menu_Bar.
 * Added the CLayoutClient interface and preferred size computations.
 */
class CMenuBar : public Fl_Menu_Bar, public CLayoutClient {
public:

    /**
     * Default constructor
     */
    CMenuBar() : Fl_Menu_Bar(0,0,10,10), CLayoutClient(this,10,SP_ALIGN_TOP) {
        box(FL_THIN_UP_BOX);
    }

    /**
     * Computes the optimal menu size
     * @param w int&, input - width offered by the program, output - width required by widget
     * @param h int&, input - height offered by the program, output - height required by widget
     */
    virtual bool preferredSize(int& w,int& h) {
        h = textsize() + 10;
        return false;
    }

    /**
     * @brief Creates a widget based on the XML node information
     * @param node xml::Node*, an XML node with widget information
     */
    static CLayoutClient* creator(xml::Node* node) {
       CMenuBar* widget = new CMenuBar;
       widget->load(node,LXM_LAYOUTDATA);
       return widget;
    }

    /**
     * @brief Returns widget class name (internal SPTK RTTI).
     */
    virtual String className() const {
        return "menu";
    }
};
/**
 * @}
 */
}

#endif
