/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CDropDownBox.h - description                           ║
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

#ifndef __CDROPDOWNBOX_H__
#define __CDROPDOWNBOX_H__

#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <sptk5/gui/CPopupWindow.h>
#include <sptk5/gui/CDBListView.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

class CDatabase;
class QueryParameter;

/**
 * @brief Drop-down list view.
 *
 * A special derivation of CDBListView to support CDBDropDownList class.
 */
class SP_EXPORT CDBDropDownListView : public CDBListView
{

protected:

    /**
     * Internal mouse click callback
     */
    void item_clicked(int mouse_btn);

    /**
     * Internal focus exits callback
     */
    void onExit();

public:

    /**
     * Constructor in SPTK style
     * @param label             Label
     * @param layoutSize        Widget size in layout
     * @param layoutAlign       Widget align in layout
     */
    CDBDropDownListView(const char* label, int layoutSize, CLayoutAlign layoutAlign)
            : CDBListView(label, layoutSize, layoutAlign)
    {}

#ifdef __COMPATIBILITY_MODE__
    /**
     * Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     */
    CDBDropDownListView(int x,int y,int w,int h) : CDBListView(x,y,w,h) {}
#endif

};

/**
 * @brief Drop-down window with CListView widget inside.
 *
 * Designed specifically for CComboBox.
 */
class SP_EXPORT CDBDropDownList : public CPopupWindow
{
    friend class CBaseListBox;
    friend class CDBDropDownListView;

protected:
    /**
     * Special version of handle() function
     */
    int handle(int) override;

public:
    /**
     * Modified list view widget
     */
    CDBDropDownListView* listView;

    /**
     * Constructor
     * @param width         Window width
     * @param height        Window height
     * @param label         Window caption
     */
    CDBDropDownList(int width, int height, const char* label = 0);

    /**
     * Resizes window and internal widgets
     * @param x             X-position
     * @param y             Y-position
     * @param w             Hidth
     * @param h             Height
     */
    void resize(int x, int y, int w, int h) override;

    /**
     * Computes the preferred size of the popup window
     * @param w             Input - width offered by the program, output - width required by widget
     * @param h             Input - height offered by the program, output - height required by widget
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    bool preferredSize(int& w, int& h) override;
};
/**
 * @}
 */
}
#endif
