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

#include <FL/Fl_Group.H>
#include <sptk5/Strings.h>
#include <sptk5/gui/CLayoutClient.h>
#include <sptk5/gui/CLayoutManager.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief SPTK group widget.
 *
 * Extended version of FLTK Fl_Group that can be a layout manager and layout client
 * at the same time.
 */
class SP_EXPORT CGroup
    : public Fl_Group, public CLayoutManager
{
    /**
     * Draw the contents of the group clipped inside the group
     */
    bool m_drawClipped;
public:

    /**
     * @returns true if contents of the group is drawn clipped inside the group
     */
    bool drawClipped() const;

    /**
     * Set that contents of the group is drawn clipped or not inside the group
     * @param drawClipped        True if contents of the group is drawn clipped inside the group
     */
    void drawClipped(bool drawClipped);

protected:


    /**
     * Constructor initializer
     * @param label const char *, label
     */
    void ctor_init(const char* label);

public:

    /**
     * Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    explicit CGroup(const char* label = nullptr, int layoutSize = 10,
                    CLayoutAlign layoutAlign = CLayoutAlign::TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CGroup(int x,int y,int w,int h,const char *label=0L);
#endif

    /**
     * Draws the group
     */
    void draw() override;

    /**
     * Resizes the group and inside widgets.
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     */
    void resize(int x, int y, int w, int h) override;

    /**
     * Computes the optimal group size
     * @param w int&, input - width offered by the program, output - width required by widget
     * @param h int&, input - height offered by the program, output - height required by widget
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    bool preferredSize(int& w, int& h) override;

    /**
     * @brief Removes all the widgets inside the group
     */
    void clear() override
    {
        Fl_Group::clear();
    }

    /**
     * @brief Returns the current label
     */
    const String& label() const override
    {
        return m_label;
    }

    using CLayoutClient::label;

    /**
     * @brief Creates a widget based on the XML node information
     * @param node              an XML node with widget information
     */
    static CLayoutClient* creator(const xdoc::SNode& node);

    /**
     * @brief Returns widget class name (internal SPTK RTTI).
     */
    String className() const override
    {
        return "group";
    }
};
/**
 * @}
 */
}
