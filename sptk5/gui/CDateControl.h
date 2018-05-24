/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CDateControl.h - description                           ║
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

#ifndef __CDATECONTROL_H__
#define __CDATECONTROL_H__

#include <FL/Fl_Group.H>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

class CSmallButton;
class CInput_;

/**
 * @brief Date input widget.
 *
 * A masked date input with a small drop-down button inside.
 * Clicking on the button displays the calendar window.
 */
class CDateControl : public Fl_Group {

    /**
     * Drop-down button
     */
    CSmallButton   *m_button;

    /**
     * Input box
     */
    CInput_        *m_input;

protected:

    /**
     * Internal callback
     */
    static void date_cb(Fl_Widget *w,void *data);
public:

    /**
     * Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label const char *, label
     */
    CDateControl(int x,int y,int w,int h,const char *label=0L);

    /**
     * Returns internal small button pointer
     */
    CSmallButton *button() {
        return m_button;
    }

    /**
     * Returns internal input box
     */
    CInput_ *input() {
        return m_input;
    }

    /**
     * Resizes the control.
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     */
    void resize(int x,int y,int w,int h);

    /**
     * Computes the best size for the control
     * @param w int&, control width
     * @param h int&, control height
     * @param editorWidth int& the width of date input
     * @param buttonWidth int& the width of button
     */
    void preferredSize(int& w,int& h,int& editorWidth,int& buttonWidth);

    /**
     * Computes the best size for the control
     * @param h int&, control height
     */
    void preferredHeight(int& h);
};
/**
 * @}
 */
}
#endif
