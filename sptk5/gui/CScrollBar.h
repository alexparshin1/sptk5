/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CScrollBar.h - description                             ║
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

#ifndef __CSCROLLBAR_H__
#define __CSCROLLBAR_H__

#include <FL/Fl_Scrollbar.H>
#include <sptk5/sptk.h>
#include <sptk5/gui/CLayoutClient.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * Extended version of Scroll Bar - to add theme support
 */
class SP_EXPORT CScrollBar : public Fl_Scrollbar, public CLayoutClient {
protected:
    /**
     * flag to detect pushed button
     */
    int m_pushed;


    /**
     * Custom draw method to support SPTK themes
     */
    void draw();
public:
    /**
     * Constructor in SPTK style
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CScrollBar(int layoutSize=16,CLayoutAlign layoutAlign=SP_ALIGN_RIGHT)
            : Fl_Scrollbar(0,0,10,10), CLayoutClient(this,layoutSize,layoutAlign) {
        m_pushed = 0;
    }

    /**
     * Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     */
    CScrollBar(int x,int y,int w,int h)
            : Fl_Scrollbar(x,y,w,h), CLayoutClient(this,10,SP_ALIGN_NONE) {
        m_pushed = 0;
    }

    /**
     * Computes the optimal progress bar size
     * @param w int&, input - width offered by the program, output - width required by widget
     * @param h int&, input - height offered by the program, output - height required by widget
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    virtual bool preferredSize(int& w,int& h);

    /**
     * Custom handle() method - event processing
     */
    int handle(int event);

    /**
     * @brief Returns widget class name (internal SPTK RTTI).
     */
    virtual String className() const {
        return "scroll_bar";
    }
};
/**
 * @}
 */
}

#endif
