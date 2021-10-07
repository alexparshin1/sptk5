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

#include <sptk5/sptk.h>

#include <FL/Fl_Box.H>
#include <sptk5/gui/CControl.h>
#include <sptk5/gui/CInput.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * Simple text viewer
 *
 * Multiple line text box (read-only)
 */
class SP_EXPORT CBox
    : public CInput
{
    using inherited = class CInput;

    /**
     * X-coordinate when mouse was pushed down
     */
    int m_xPushed {0};

    /**
     * Y-coordinate when mouse was pushed down
     */
    int m_yPushed {0};

    /**
     * Flag that allows to drag a window, if true
     */
    bool m_dragable {false};


    /**
     * Constructor initializer
     */
    void ctor_init(const char* label);

public:
    /**
     * Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CBox(const char* label = nullptr, int layoutSize = 10, CLayoutAlign layoutAlign = CLayoutAlign::TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CBox(int x, int y, int w, int h, const char* label = 0);
#endif

    /**
     * Universal data connection, returns data from control
     */
    Variant data() const override;

    /**
     * Universal data connection, sets data from control
     */
    void data(const Variant& v) override;

    /**
     * Returns the control kind, SPTK-style RTTI
     * @see CControlKind for more information
     */
    CControlKind kind() const override
    {
        return CControlKind::BOX;
    }

    /**
     * Returns the control class name, SPTK-style RTTI
     */
    String className() const override
    {
        return "box";
    }

    /**
     * Returns the text font type
     */
    Fl_Font textFont() const override;

    /**
     * Sets the text font type
     */
    void textFont(Fl_Font f) override;

    /**
     * Returns the text font size
     */
    uchar textSize() const override;

    /**
     * Sets the input text font size
     */
    void textSize(uchar s) override;

    /**
     * Returns the text alignment.
     */
    virtual uint32_t textAlign() const;

    /**
     * Sets the text alignment
     */
    virtual void textAlign(uint32_t align);

    /**
     * Computes totalHeight for the text inside
     */
    virtual int totalHeight(int ww = 0) const;

    /**
     * Custom draw method
     */
    void draw() override;

    /**
     * Computes the optimal widget size
     * @param w int&, input - width offered by the program, output - width required by widget
     * @param h int&, input - height offered by the program, output - height required by widget
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    bool preferredSize(int& w, int& h) override;

    /**
     * Resizes the control and inside widgets.
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     */
    void resize(int x, int y, int w, int h) override;

    /**
     * Creates a widget based on the XML node information
     */
    static CLayoutClient* creator(const xdoc::SNode& node);

    /**
     * Custom handle() to support drag event
     */
    int handle(int event) override;

    /**
     * Returns flag that allows to drag a window, if true
     */
    bool dragable() const
    {
        return m_dragable;
    }

    /**
     * Sets flag that allows to drag a window, if true
     */
    void dragable(bool df)
    {
        m_dragable = df;
    }
};
/**
 * @}
 */
} // namespace sptk
