/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CCheckButtons.h - description                          ║
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

#ifndef __CCheckButtons_H__
#define __CCheckButtons_H__

#include <FL/Fl_Check_Button.H>
#include <sptk5/gui/CLayoutClient.h>
#include <sptk5/gui/CRadioButtons.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief The check button.
 *
 * Implements the SPTK check button that understands and reports
 * its size requirements, and can be a layout client.
 */
class SP_EXPORT CCheckButton : public Fl_Check_Button, public CLayoutClient {

protected:

    /**
     * Draws the check button
     */
    virtual void draw();

public:

    /**
     * SPTK-style constructor
     * @param label const char *, the widget label
     * @param layoutSize int, the size of widget in layout
     * @param layoutAlignment CLayoutAlign, widget align in the layout
     */
    CCheckButton(const char *label,int layoutSize,CLayoutAlign layoutAlignment)
            : Fl_Check_Button(0,0,10,10,label), CLayoutClient(this,layoutSize,layoutAlignment) {}

    /**
     * A special handle method. See FLTK Fl_Widget::handle() for more information.
     * @param event int, FLTK event type
     * @return true if the event was processed
     */
    int handle(int event);

    /**
     * Reports the preferred size of the widget based on offered size.
     * @param w int, input/output desirable widget width
     * @param h int, input/output desirable widget heigth
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    virtual bool preferredSize(int& w,int& h);
};

/**
 * @brief A set of check buttons.
 *
 * Implements a group of check buttons with an optional CInput for the 'Other' choice.
 * Data connector returns the current state as a string with selected buttons names separated with '|' character.
 */
class SP_EXPORT CCheckButtons : public CRadioButtons {
    static void checkButtonsCallback(Fl_Widget *,void *);
protected:
    virtual Fl_Button *createButton(const char *label,int sz=10,CLayoutAlign layoutAlignment=SP_ALIGN_TOP);
public:

    /**
     * SPTK-style constructor
     * @param label const char *, the widget label
     * @param layoutSize int, the size of widget in layout
     * @param layoutAlignment CLayoutAlign, widget align in the layout
     */
    CCheckButtons(const char * label=0,int layoutSize=20,CLayoutAlign layoutAlignment=SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * FLTK-style constructor
     * @param x int, widget x-coordinate
     * @param y int, widget y-coordinate
     * @param w int, widget width
     * @param h int, widget height
     * @param label int, optional widget label
     */
    CCheckButtons(int,int,int,int,const char * = 0);
#endif

    /**
     * Gets the currently selected buttons argument()s
     */
    void getSelections(IntList& selection) const;

    /**
     * Sets the currently selected buttons matching with argument()s
     */
    void setSelections(const IntList& selection);

    /**
     * Universal data connection reader.
     * @returns the list of selected button labels as pipe ('|') separated strings
     */
    virtual Variant data() const;

    /**
     * Universal data connection writer.
     * @param v CVariant, sets the selection from the list of labels as pipe ('|') separated strings
     */
    virtual void     data(const Variant v);

    /**
     * Returns an internal SPTK control type information
     */
    virtual CControlKind kind() const {
        return DCV_CHECKBUTTONS;
    }

    /**
     * Returns an internal SPTK control class name information
     */
    virtual String className() const {
        return "check_buttons";
    }
    
    /**
     * @brief Creates chack buttons group based on the XML node information
     */
    static CLayoutClient* creator(xml::Node *node);
};
/**
 * @}
 */
}
#endif
