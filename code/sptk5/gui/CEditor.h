/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#pragma once

#include <FL/Fl_Text_Editor.H>

#include <sptk5/gui/CLayoutClient.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Text editor widget
 *
 * Class CEditor is the Fl_Text_Editor combined with the layout client.
 * It has a modified handle() function to track the current cursor position.
 * It also allocates the text buffer in constructor and destroys it in destructor.
 */
class SP_EXPORT CEditor
    : public Fl_Text_Editor, public CLayoutClient
{

    /**
     * Constructor initializer
     */
    void ctor_init();

    /**
     * Last cursor position in bytes, from the beginning of the text buffer
     */
    int m_lastCursorPosition;

    /**
     * Current wrap mode
     */
    bool m_wrapMode;

public:

    /**
     * Constructor in SPTK style
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    explicit CEditor(int layoutSize = 100, CLayoutAlign layoutAlign = CLayoutAlign::TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CEditor(int x, int y, int w, int h, const char *label);
#endif

    /**
     * Destructor
     */
    ~CEditor() override;

    /**
     * Retruns internal text buffer
     */
    Fl_Text_Buffer* textBuffer()
    {
        return mBuffer;
    }

    /**
     * Retruns internal style buffer
     */
    Fl_Text_Buffer* styleBuffer()
    {
        return mStyleBuffer;
    }

    /**
     * Computes the optimal widgets size
     * @param w int&, input - width offered by the program, output - width required by widget
     * @param h int&, input - height offered by the program, output - height required by widget
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    bool preferredSize(int& w, int& h) override;

    /**
     * Returns current cursor position
     * @param row int&, row number
     * @param col int&, column number
     */
    void cursorRowCol(int& row, int& col);

    /**
     * Returns current wrap mode
     */
    bool wrapMode() const
    {
        return m_wrapMode;
    }

    /**
     * Sets current wrap mode
     */
    void wrapMode(bool wm);

    /**
     * Special version of handle() function
     */
    int handle(int event) override;
};
/**
 * @}
 */
}
