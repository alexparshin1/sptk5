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

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Input_.H>
#include <sptk5/Strings.h>
#include <sptk5/gui/CControl.h>
#include <sptk5/gui/CInput.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * Base class for CCheckButtons and CRadioButtons.
 *
 * Implements the most important data communication methods for these button groups.
 */
class SP_EXPORT CButtonGroup
    : public CControl
{
    /**
     * Required group height
     */
    int m_maxHeight {0};

    /**
     * Last value of the group (last list of choices)
     */
    std::string m_lastValue;

    /**
     * Button labels for the buttons inside
     */
    Strings m_buttonLabels;

    /**
     * The 'Other' Button if requested (add '*' in the button list)
     */
    Fl_Button* m_otherButton {nullptr};

    /**
     * The 'Other' Input if requested (add '*' in the button list)
     */
    CInput_* m_otherInput {nullptr};

protected:
    /**
     * Internal callback processing
     */
    virtual void controlDataChanged();

    /**
     * Finds a button by label
     * @param buttonLabel const char *, button label
     * @returns button index, or -1 if not found
     */
    int buttonIndex(const char* buttonLabel);

    /**
     * Deselects all buttons
     */
    void deselectAllButtons();

    /**
     * Creates button. Should be implemented in the derived class.
     */
    virtual Fl_Button* createButton(const char* label, int sz = 10,
                                    CLayoutAlign layoutAlignment = CLayoutAlign::TOP) = 0;

    /**
     * Constructor initializer
     */
    void ctor_init();

    /**
     * SPTK-style constructor
     * @param label const char *, the widget label
     * @param layoutSize int, the size of widget in layout
     * @param layoutAlignment CLayoutAlign, widget align in the layout
     */
    CButtonGroup(const char* label = nullptr, int layoutSize = 20,
                 CLayoutAlign layoutAlignment = CLayoutAlign::TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * FLTK-style constructor
     * @param x int, widget x-coordinate
     * @param y int, widget y-coordinate
     * @param w int, widget width
     * @param h int, widget height
     * @param label int, optional widget label
     */
    CButtonGroup(int, int, int, int, const char* = 0);
#endif

public:
    /**
     * Sets the list of the buttons.
     * @param buttonList        list of the buttons
     */
    void buttons(const Strings& buttonList);

    /**
     * Returns the list of the buttons.
     */
    const Strings& buttons() const
    {
        return m_buttonLabels;
    }

    /**
     * Clears the list of buttons.
     */
    virtual void clearButtons();

    /**
     * Returns the currently selected button(s) as pipe ('|') separated string
     */
    Variant data() const override;

    /**
     * Sets the currently selected button(s)
     *
     * Buttons are presented as pipe ('|') separated string.
     * If the button group allows only one button to be selected at a time
     * (like radio buttons), only the first item of the string will be used.
     */
    void data(const Variant& v) override;

    /**
     * Loads the the currently selected button(s)
     *
     * Buttons should be presented as pipe ('|') separated string.
     */
    void load(Query*) override;

    /**
     * Saves the the currently selected button(s)
     *
     * Buttons are presented as pipe ('|') separated string
     */
    void save(Query*) override;

    /**
     * Loads control data from XML
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node              the XML node
     * @param xmlMode           the mode defining how the layout and/or data should be stored
     */
    void load(const xdoc::SNode& node, CLayoutXMLmode xmlMode) override;

    /**
     * @brief Loads control data from XML
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node              the XML node
     */
    void load(const xdoc::SNode& node) override
    {
        load(node, CLayoutXMLmode::DATA);
    }

    /**
     * Saves control data to XML
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node              the XML node
     * @param xmlMode           the mode defining how the layout and/or data should be stored
     */
    void save(const xdoc::SNode& node, CLayoutXMLmode xmlMode) const override;

    /**
     * @brief Saves control data to XML
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node              the XML node
     */
    virtual void save(const xdoc::SNode& node) const
    {
        save(node, CLayoutXMLmode::DATA);
    }

    /**
     * Tells if the the current data content is valid
     *
     * Always true for this widget.
     */
    bool valid() const override
    {
        return true;
    }

    /**
     * Computes the preferred size of the button group based on its contents
     * @param w                 the optimal width
     * @param h                 the optimal height
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    bool preferredSize(int& w, int& h) override;

    /**
     * The 'Other' Button if requested (add '*' in the button list)
     */
    Fl_Button* otherButton() const
    {
        return m_otherButton;
    }

    /**
     * The 'Other' Input if requested (add '*' in the button list)
     */
    CInput_* otherInput() const
    {
        return m_otherInput;
    }
};
/**
 * @}
 */
} // namespace sptk
