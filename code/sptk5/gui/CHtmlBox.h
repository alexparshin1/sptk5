/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __HTMLBOX_H__
#define __HTMLBOX_H__

#include <sptk5/sptk.h>

#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input_.H>
#include <sptk5/gui/CInput.h>
#include <sptk5/gui/CControl.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Simple HTML viewer
 *
 * Multiple line HTML text box (read-only)
 */
class SP_EXPORT CHtmlBox : public CInput
{
    typedef class CInput inherited;

    /**
     * @brief Constructor initializer
     */
    void ctor_init(const char* label);

public:

    /**
     * @brief Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    explicit CHtmlBox(const char* label = nullptr, int layoutSize = 10, CLayoutAlign layoutAlign = SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * @brief Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CHtmlBox(int,int,int,int,const char * = 0);
#endif

    /**
     * @brief Universal data connection, returns data from control
     */
    Variant data() const override;

    /**
     * @brief Universal data connection, sets data from control
     */
    void data(const Variant v) override;

    /**
     * @brief Returns the control kind, SPTK-style RTTI
     * @see CControlKind for more information
     */
    CControlKind kind() const override
    {
        return DCV_HTMLBOX;
    }

    /**
     * @brief Returns the control class name, SPTK-style RTTI
     */
    String className() const override
    {
        return "html";
    }

    /**
     * @brief Returns the input text font type
     */
    Fl_Font textFont() const override;

    /**
     * @brief Sets the input text font type
     */
    void textFont(Fl_Font f) override;

    /**
     * @brief Returns the input text font size
     */
    uchar textSize() const override;

    /**
     * @brief Sets the input text font size
     */
    void textSize(uchar s) override;

    /**
     * @brief Computes the total height of HTML text inside if the widgets height isn't limited
     */
    int totalHeight() const;

    /**
     * @brief Computes the optimal widget size
     * @param w int&, input - width offered by the program, output - width required by widget
     * @param h int&, input - height offered by the program, output - height required by widget
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    bool preferredSize(int& w, int& h) override;

    /**
     * @brief Creates a widget based on the XML node information
     */
    static CLayoutClient* creator(xml::Node* node);
};

/**
 * @}
 */
}

#endif
