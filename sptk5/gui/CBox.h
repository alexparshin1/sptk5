/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CBox.h - description                                   ║
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

#ifndef __CBOX_H__
#define __CBOX_H__

#include <sptk5/sptk.h>

#include <FL/Fl_Box.H>
#include <sptk5/gui/CInput.h>
#include <sptk5/gui/CControl.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Simple text viewer
 *
 * Multiple line text box (read-only)
 */
class SP_EXPORT CBox : public CInput {
    typedef class CInput inherited;

    /**
     * X-coordinate when mouse was pushed down
     */
    int         m_xPushed;

    /**
     * Y-coordinate when mouse was pushed down
     */
    int         m_yPushed;

    /**
     * Flag that allows to drag a window, if true
     */
    bool        m_dragable;


    /**
     * @brief Constructor initializer
     */
    void ctor_init(const char *label);

public:

    /**
     * @brief Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CBox(const char * label=0,int layoutSize=10,CLayoutAlign layoutAlign=SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * @brief Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CBox(int x,int y,int w,int h,const char *label = 0);
#endif

    /**
     * @brief Universal data connection, returns data from control
     */
    virtual Variant data() const;

    /**
     * @brief Universal data connection, sets data from control
     */
    virtual void     data(const Variant v);

    /**
     * @brief Returns the control kind, SPTK-style RTTI
     * @see CControlKind for more information
     */
    virtual CControlKind kind() const {
        return DCV_BOX;
    }

    /**
     * @brief Returns the control class name, SPTK-style RTTI
     */
    virtual String className() const {
        return "box";
    }

    /**
     * @brief Returns the text font type
     */
    virtual Fl_Font  textFont() const;

    /**
     * @brief Sets the text font type
     */
    virtual void     textFont(Fl_Font f);

    /**
     * @brief Returns the text font size
     */
    virtual uchar    textSize() const;

    /**
     * @brief Sets the input text font size
     */
    virtual void     textSize(uchar s);

    /**
     * @brief Returns the text alignment.
     */
    virtual uint32_t textAlign() const;

    /**
     * @brief Sets the text alignment
     */
    virtual void     textAlign(uint32_t align);

    /**
     * @brief Computes totalHeight for the text inside
     */
    virtual int      totalHeight(int ww=0) const;

    /**
     * @brief Custom draw method
     */
    virtual void     draw();

    /**
     * @brief Computes the optimal widget size
     * @param w int&, input - width offered by the program, output - width required by widget
     * @param h int&, input - height offered by the program, output - height required by widget
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    virtual bool preferredSize(int& w,int& h);

    /**
     * @brief Resizes the control and inside widgets.
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     */
    virtual void     resize(int x,int y,int w,int h);

    /**
     * @brief Creates a widget based on the XML node information
     */
    static CLayoutClient* creator(xml::Node *node);

    /**
     * @brief Custom handle() to support drag event
     */
    int handle(int event);

    /**
     * @brief Returns flag that allows to drag a window, if true
     */
    bool dragable() const { return m_dragable; }

    /**
     * @brief Sets flag that allows to drag a window, if true
     */
    void dragable(bool df) { m_dragable = df; }
};
/**
 * @}
 */
}
#endif
