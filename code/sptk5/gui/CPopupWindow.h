/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CPopupWindow.h - description                           ║
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

#ifndef __CPOPUPWINDOW_H__
#define __CPOPUPWINDOW_H__

#include <sptk5/gui/CWindow.h>
#include <sptk5/sptk.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Base popup window
 *
 * The CWindow descendant that closes the window if the user
 * clicks outside of it with mouse or moves the focus outside
 * the window any other way
 */
class SP_EXPORT CPopupWindow : public CWindow {
protected:
    /**
     * The outside mouse click
     */
    int  m_clicked;
    /**
     * The special handle() function
     */
    int  handle(int);
public:
    /**
     * Constructor
     * @param w int, window width
     * @param h int, window height
     * @param label int, window label
     */
    CPopupWindow(int w, int h, const char *label = 0);

    /**
     * Shows window in modal mode. Exits from the modal mode
     * when the focus is moved outside the window.
     */
    bool showModal();
};
/**
 * @}
 */
}
#endif
