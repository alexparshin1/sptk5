/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CThemeScrollBar.h - description                        ║
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

#ifndef __CTHEMESCROLLBAR_H__
#define __CTHEMESCROLLBAR_H__

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <sptk5/cxml>
#include <sptk5/gui/CThemeImageCollection.h>
#include <map>
#include <string>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Theme scroll bar type - defines the scroll bar direction
 */

enum CThemeScrollBarType
{
    THM_SCROLLBAR_VERTICAL, THM_SCROLLBAR_HORIZONTAL
};

/**
 * @brief The complete set of four images to draw a scroll bar
 */

class CScrollBarImages
{
public:

    /**
     * @brief Type of the scroll bar image
     */
    enum type
    {
        /**
         * Image of the scroll bar without slider and buttons
         */
        TROUGH,

        /**
         * Image of the scroll bar slider
         */
        SLIDER

    };

    /**
     * Scrollbar orientation
     */
    CThemeScrollBarType m_orientation;

    /**
     * Scrollbar parts
     */
    CThemeImageCollection m_parts[3];

    /**
     * Stepper buttons, 0/1 - (left or upper)normal/pressed, 2/3 - (right or buttom)normal/pressed
     */
    CThemeImageCollection m_steppers[2];

public:

    /**
     * @brief Default constructor
     */
    CScrollBarImages() noexcept
    : m_orientation(THM_SCROLLBAR_VERTICAL)
    {
    }

    /**
     * @brief Loads the images from SPTK theme files
     * @param atype CThemeScrollBarType, scroll bar type
     * @param desaturateInactiveButtons bool, if true then inactive buttons are desaturated
     * @param externalFile bool, if true then the file belongs to GTK theme and not to SPTK theme tar
     */
    void load(CThemeScrollBarType atype, bool desaturateInactiveButtons, bool externalFile = false);

    /**
     * @brief Clears the scrollbar images for the default theme
     */
    void clear();
};

class CThemeScrollBar
{
public:
    /**
     * @brief Images for painting a scroll bar (horizontal and vertical)
     *
     * Images should have the compatible size.
     * If you put the slider image on top its frame - they should show
     * a picture of 100% progress
     */
    static CScrollBarImages m_scrollBar[2];

    /**
     * Trough border width
     */
    static int m_scrollBarTroughBorder;

    /**
     * Slider width
     */
    static int m_scrollBarSliderWidth;

    /**
     * Button width
     */
    static int m_scrollBarStepperSize;


    /**
     * @brief Loads GTK theme scrollbar button images from 'style' node
     * @param xml XMLDoc&, GTK theme converted to XML
     * @param orientation std::string, scrollbar button orientation ("UP","DOWN","LEFT","RIGHT")
     * @param buttonImages CThemeImageCollection&, output button images
     */
    static void loadGtkScrollbarButtons(
            xml::Document& xml, const String& orientation, CThemeImageCollection& buttonImages);

    /**
     * @brief Loads GTK theme scrollbar troughs
     * @param xml XMLDoc&, GTK theme converted to XML
     */
    static void loadGtkScrollbarTroughs(xml::Document& xml);

    /**
     * @brief Loads GTK theme scrollbar sliders
     * @param xml XMLDoc&, GTK theme converted to XML
     */
    static void loadGtkScrollbarSliders(xml::Document& xml);

    /**
     * @brief Loads GTK theme scrollbar button images from GTK theme
     * @param xml XMLDoc&, GTK theme converted to XML
     */
    static void loadGtkScrollbars(xml::Document& xml);

    /**
     * @brief Computes and returns the minimum size of the scroll bar
     * @param w int&, scroll bar width, may be altered by scroll bar to meet the scroll bar size requirements
     * @param h int&, scroll bar height, may be altered by scroll bar to meet the scroll bar size requirements
     * @returns false, if theme has no pixmaps for the scroll bar, or true otherwise
     */
    static bool sizeScrollBar(int& w, int& h);

    /**
     * @brief Draws a scroll bar image.
     *
     * The type of the scroll bar is defined by max of (w,h).
     * @returns false, if theme has no pixmaps for the scroll bar, or true otherwise
     * @param x int, x-coordinate of the scroll bar
     * @param y int, y-coordinate of the scroll bar
     * @param w int, width of the scroll bar
     * @param h int, width of the scroll bar
     * @param minimum float, minimum value of the scroll bar
     * @param maximum float, maximum value of the scroll bar
     * @param value float, value of the scroll bar
     * @param size float, size of the scroll bar
     * @param activeZone int, 0 - no active zone, 1 - button 1, 2 - button 2, 3 - slider, +8 - pushed
     * @param active bool, true if scrollbar is active (enabled)
     */
    static bool drawScrollBar(int x, int y, int w, int h, float minimum, float maximum, float value, float size, int activeZone, bool active);
};

/**
 * @}
 */
}

#endif
