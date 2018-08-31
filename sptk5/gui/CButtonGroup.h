/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CButtonGroup.h - description                           ║
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

#ifndef __CButtonGroup_H__
#define __CButtonGroup_H__

#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Input_.H>
#include <FL/Fl_Button.H>
#include <sptk5/IntList.h>
#include <sptk5/gui/CControl.h>
#include <sptk5/gui/CInput.h>
#include <sptk5/Strings.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Base class for CCheckButtons and CRadioButtons.
 *
 * Implements the most important data communication methods for these button groups.
 */
class SP_EXPORT CButtonGroup : public CControl {
    /**
     * Required group height
     */
    int         m_maxHeight;

protected:
    /**
     * Last value of the group (last list of choices)
     */
    std::string m_lastValue;

    /**
     * Button labels for the buttons inside
     */
    Strings     m_buttonLabels;

    /**
     * The 'Other' Button if requested (add '*' in the button list)
     */
    Fl_Button  *m_otherButton;

    /**
     * The 'Other' Input if requested (add '*' in the button list)
     */
    CInput_    *m_otherInput;


    /**
     * Internal callback processing
     */
    virtual void controlDataChanged();

    /**
     * Finds a button by label
     * @param buttonLabel const char *, button label
     * @returns button index, or -1 if not found
     */
    int  buttonIndex(const char *buttonLabel);

    /**
     * Deselects all buttons
     */
    void deselectAllButtons();

    /**
     * Selects button by index
     */
    void selectButton(int choice);

    /**
     * Creates button. Should be implemented in the derived class.
     */
    virtual Fl_Button *createButton(const char *label,int sz=10,CLayoutAlign layoutAlignment=SP_ALIGN_TOP) = 0;

    /**
     * Constructor initializer
     */
    void ctor_init();

protected:

    /**
     * SPTK-style constructor
     * @param label const char *, the widget label
     * @param layoutSize int, the size of widget in layout
     * @param layoutAlignment CLayoutAlign, widget align in the layout
     */
    CButtonGroup(const char * label=0,int layoutSize=20,CLayoutAlign layoutAlignment=SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * FLTK-style constructor
     * @param x int, widget x-coordinate
     * @param y int, widget y-coordinate
     * @param w int, widget width
     * @param h int, widget height
     * @param label int, optional widget label
     */
    CButtonGroup(int,int,int,int,const char * = 0);
#endif

public:
    /**
     * @brief Sets the list of the buttons.
     * @param buttonList Strings, list of the buttons
     */
    void buttons(const Strings& buttonList);

    /**
     * @brief Returns the list of the buttons.
     */
    const Strings& buttons() const { return m_buttonLabels; }

    /**
     * @brief Clears the list of buttons.
     */
    virtual void clearButtons();

    /**
     * @brief Returns the currently selected button(s) as pipe ('|') separated string
     */
    virtual Variant data() const;

    /**
     * @brief Sets the currently selected button(s)
     *
     * Buttons are presented as pipe ('|') separated string.
     * If the button group allows only one button to be selected at a time
     * (like radio buttons), only the first item of the string will be used.
     */
    virtual void data(const Variant v);

    /**
     * @brief Loads the the currently selected button(s)
     *
     * Buttons should be presented as pipe ('|') separated string.
     */
    virtual void load(Query *);

    /**
     * @brief Saves the the currently selected button(s)
     *
     * Buttons are presented as pipe ('|') separated string
     */
    virtual void save(Query *);

    /**
     * @brief Loads control data from XML
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node xml::Node*, the XML node
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be stored
     */
    virtual void     load(const xml::Node* node,CLayoutXMLmode xmlMode);

    /**
     * @brief Saves control data to XML
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node xml::Node*, the XML node
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be stored
     */
    virtual void save(xml::Node* node,CLayoutXMLmode xmlMode) const;

    /**
     * @brief Tells if the the current data content is valid
     *
     * Always true for this widget.
     */
    virtual bool valid() const {
        return true;
    }

    /**
     * @brief Computes the preferred size of the button group based on its contents
     * @param w int&, the optimal width 
     * @param h int&, the optimal height 
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    virtual bool preferredSize(int& w,int& h);
};
/**
 * @}
 */
}
#endif
