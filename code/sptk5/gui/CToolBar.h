/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CToolBar.h - description                               ║
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

#ifndef __CTOOLBAR_H__
#define __CTOOLBAR_H__

#include <sptk5/gui/CGroup.h>
#include <sptk5/gui/CButton.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Toolbar widget
 *
 * The slightely extended CGroup to work as a toolbar.
 * Most of the toolbar functionality isn't implemented yet.
 */
class SP_EXPORT CToolBar : public CGroup {
    /**
     * Flag, true if the toolbar is collapsed
     */
    bool    m_colapsed;

public:

    /**
     * @brief Constructor in SPTK style
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    explicit CToolBar(CLayoutAlign layoutAlign=SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * @brief Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     */
    CToolBar(int x,int y,int w,int h);
#endif

    /**
     * @brief Creates and adds a new button to the toolbar
     * @param buttonKind CButtonKind, button type
     * @param cb Fl_Callback_p, button callback
     * @param label const char *, button label
     * @param buttonAlign CLayoutAlign, button align in the toolbar
     * @see CButton
     */
    CButton *addButton(CButtonKind buttonKind,Fl_Callback_p cb,const char *label = 0,CLayoutAlign buttonAlign=SP_ALIGN_LEFT);

    /**
     * @brief Resizes the group and inside widgets.
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     */
    virtual void resize(int x,int y,int w,int h);

    /**
     * @brief Computes the optimal group size
     * @param w int&, input - width offered by the program, output - width required by widget
     * @param h int&, input - height offered by the program, output - height required by widget
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    virtual bool preferredSize(int& w,int& h);

    /**
     * @brief Flag, true if the toolbar is collapsed
     */
    bool collapsed() const {
        return m_colapsed;
    }

    /**
     * @brief Special handle() function
     */
    int handle(int);

    /**
     * @brief Draws the toolbar
     */
    void draw();

    /**
     * @brief Creates a widget based on the XML node information
     * @param node xml::Node*, an XML node with widget information
     */
    static CLayoutClient* creator(xml::Node* node);

    /**
     * @brief Returns widget class name (internal SPTK RTTI).
     */
    virtual String className() const {
        return "toolbar";
    }
};
/**
 * @}
 */
}
#endif
