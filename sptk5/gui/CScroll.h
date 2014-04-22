/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CScroll.h  -  description
                             -------------------
    begin                : Sun May 22 2003
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#ifndef __CSCROLL_H__
#define __CSCROLL_H__

#include <FL/Fl_Group.H>

#include <sptk5/gui/CScrollBar.h>
#include <sptk5/gui/CLayoutClient.h>
#include <sptk5/gui/CLayoutManager.h>

namespace sptk {

/// @addtogroup gui GUI Classes
/// @{

/// @brief Different modes for scrollbars
enum CScrollbarMode { // values for type()
    HORIZONTAL = 1,
    VERTICAL = 2,
    BOTH = 3,
    ALWAYS_ON = 4,
    HORIZONTAL_ALWAYS = 5,
    VERTICAL_ALWAYS = 6,
    BOTH_ALWAYS = 7
};

/// @brief Scroll area widget
///
/// Extended version of FLTK Fl_Group that can be a layout manager and layout client
/// at the same time.
class CScroll : public ::Fl_Group, public CLayoutManager {

    int m_xposition;        ///< Current x-position of the scrolled area
    int m_yposition;        ///< Current y-position of the scrolled area
    int m_width;            ///< Current width of the scrolled area
    int m_height;           ///< Current height of the scrolled area
    int m_oldx;             ///< Former x-position of the scrolled area
    int m_oldy;             ///< Former y-position of the scrolled area

    /// @brief Horizontal scrollbar callback
    static void hscrollbar_cb(Fl_Widget*, void*);

    /// @brief Vertical scrollbar callback
    static void scrollbar_cb(Fl_Widget*, void*);

    /// @brief Constructor initializer
    void ctor_init();

    /// @brief Insure the scrollbars are the last children:
    void fix_scrollbar_order();

    /// @brief Draw the visible area. This is also the callback for fl_scroll.
    static void draw_clip(void* v,int X, int Y, int W, int H);

    /// @brief Computed area for widgets
    void bbox(int& X, int& Y, int& W, int& H);

public:

    /// @brief Constructor in SPTK style
    /// @param label const char *, label
    /// @param layoutSize int, widget align in layout
    /// @param layoutAlign CLayoutAlign, widget align in layout
    CScroll(const char * label=0,int layoutSize=10,CLayoutAlign layoutAlign=SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /// @brief Constructor in FLTK style
    /// @param x int, x-position
    /// @param y int, y-position
    /// @param w int, width
    /// @param h int, height
    /// @param label, const char * label
    CScroll(int x,int y,int w,int h,const char *label=0L);
#endif

    CScrollBar scrollbar;     ///< Vertical scrollbar
    CScrollBar hscrollbar;    ///< Horizontal scrollbar

    /// @brief Resizes the group and inside widgets.
    /// @param x int, x-position
    /// @param y int, y-position
    /// @param w int, width
    /// @param h int, height
    virtual void resize(int x,int y,int w,int h);

    /// @brief Custom draw() method
    virtual void draw();

    /// @brief Computes the optimal group size
    /// @param w int&, input - width offered by the program, output - width required by widget
    /// @param h int&, input - height offered by the program, output - height required by widget
    /// @returns true if the size is stable (doesn't depend on input sizes)
    virtual bool preferredSize(int& w,int& h);

    /// @brief Removes all the child widgets
    virtual void clear();

    /// @brief Sets the position of the scrolled area
    virtual void position(int X, int Y);

    /// @brief Returns the x-position of the scrolled area
    int xposition() const {
        return m_xposition;
    }

    /// @brief Returns the y-position of the scrolled area
    int yposition() const {
        return m_yposition;
    }

    /// @brief Custom handle method
    int handle(int);

    /// @brief Returns the current label
    std::string label() const {
        return m_label;
    }

    /// @brief Sets the new label
    ///
    /// @param lbl const char*, new label
    void label(const char* lbl) {
        CLayoutClient::label(lbl);
    }

    /// Sets label for the group, makes an internal copy of the string
    /// @param lbl const string&, new label
    void label(const std::string& lbl) {
        CLayoutClient::label(lbl);
    }

    /// @brief Creates a widget based on the XML node information
    /// @param node CXmlNode*, an XML node with widget information
    static CLayoutClient* creator(CXmlNode* node);

    /// @brief Returns widget class name (internal SPTK RTTI).
    virtual std::string className() const {
        return "scroll";
    }
};
/// @}
}
#endif
