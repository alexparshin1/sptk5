/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CRadioButtons.h - description                          ║
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

#ifndef __CRadioButtons_H__
#define __CRadioButtons_H__

#include <sptk5/sptk.h>
#include <sptk5/gui/CButtonGroup.h>
#include <string>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Radio buttons widget.
 *
 * A group of radio buttons with an optional
 * edit entry for the 'Other' choice.
 */
class SP_EXPORT CRadioButtons : public CButtonGroup {
    /**
     * Internal callback function
     */
    static void radioButtonsCallback(Fl_Widget *,void *);
protected:
    /**
     * Creates a radio button and adds it to the group
     */
    virtual Fl_Button *createButton(const char *label,int sz=10,CLayoutAlign layoutAlignment=SP_ALIGN_TOP);
public:
    /**
     * SPTK-style constructor
     * @param label const char *, the widget label
     * @param layoutSize int, the size of widget in layout
     * @param layoutAlignment CLayoutAlign, widget align in the layout
     */
    CRadioButtons(const char * label=0,int layoutSize=20,CLayoutAlign layoutAlignment=SP_ALIGN_TOP);
#ifdef __COMPATIBILITY_MODE__
    /**
     * FLTK-style constructor
     * @param x int, widget x-coordinate
     * @param y int, widget y-coordinate
     * @param w int, widget width
     * @param h int, widget height
     * @param label int, optional widget label
     */
    CRadioButtons(int,int,int,int,const char * = 0);
#endif
    /**
     * SPTK internal control type
     */
    virtual CControlKind kind() const {
        return DCV_RADIOBUTTONS;
    }

    /**
     * SPTK internal class name
     */
    virtual String className() const {
        return "radio_buttons";
    }

    /**
     * Gets the selected button's ID
     */
    int32_t intValue() const;

    /**
     * Sets the selected button by ID
     */
    void intValue(int32_t ID);

    /**
     * @brief Creates a widget based on the XML node information
     * @param node xml::Node*, an XML node with widget information
     */
    static CLayoutClient* creator(xml::Node* node);
};
/**
 * @}
 */
}
#endif
