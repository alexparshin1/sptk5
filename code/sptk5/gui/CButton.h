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
#include <sptk5/gui/CLayoutClient.h>
#include <sptk5/gui/CIcon.h>
#include <sptk5/gui/CThemes.h>
#include <string>

#include <FL/Fl_Button.H>

class Fl_Image;

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * Button widget kind
 *
 * The button kind defines a picture and a default label for the button.
 */
enum class CButtonKind
    : uint32_t
{
    UNDEFINED_BUTTON = 0,        ///< Default button without image, or with user-defined image
    OK_BUTTON = 1,               ///< 'Ok' button
    CANCEL_BUTTON = 2,           ///< 'Cancel' button
    NO_BUTTON = 4,               ///< 'No' button
    ADD_BUTTON = 8,              ///< 'Add' button
    DELETE_BUTTON = 0x10,        ///< 'Delete' button
    EDIT_BUTTON = 0x20,          ///< 'Edit' button
    BROWSE_BUTTON = 0x40,        ///< 'Browse' button
    REFRESH_BUTTON = 0x80,       ///< 'Refresh' button
    CALENDAR_BUTTON = 0x100,     ///< 'Calendar' button
    OPEN_BUTTON = 0x200,         ///< 'Open' button
    PRINT_BUTTON = 0x400,        ///< 'Print' button
    SAVE_BUTTON = 0x800,         ///< 'Save' button
    SAVE_AS_BUTTON = 0x1000,     ///< 'Save As' button
    COPY_BUTTON = 0x2000,        ///< 'Copy' button
    LEFT_BUTTON = 0x4000,        ///< 'Left' button
    NEW_BUTTON = 0x8000,         ///< 'New' button
    NEXT_BUTTON = 0x10000,       ///< 'Next' button
    PRINTER_BUTTON = 0x20000,    ///< 'Printer' button
    PRIOR_BUTTON = 0x40000,      ///< 'Prior' button
    RIGHT_BUTTON = 0x80000,      ///< 'Right' button
    SEARCH_BUTTON = 0x100000,    ///< 'Left' button
    SEND_BUTTON = 0x200000,      ///< 'Send' button
    STEPLEFT_BUTTON = 0x400000,  ///< 'Step Left' button
    STEPRIGHT_BUTTON = 0x800000, ///< 'Step Right' button
    VIEW_BUTTON = 0x1000000,     ///< 'View' button
    HOME_BUTTON = 0x2000000,     ///< 'Home' button
    CONFIGURE_BUTTON = 0x4000000,///< 'Configure' button
    EXEC_BUTTON = 0x8000000,     ///< Execute' button
    STOP_BUTTON = 0x10000000,    ///< 'Stop' button
    EXIT_BUTTON = 0x20000000,    ///< 'Exit' button
    HELP_BUTTON = 0x40000000,    ///< 'Help' button
    MAX_BUTTON = 0x40000001      ///< Max button id
};

/**
 * Base button widget
 *
 * Base class for CButton and CSmallButton, uses Fl_Image * or a stock image of CButtonKind.
 */
class SP_EXPORT CBaseButton
    : public Fl_Button, public CLayoutClient
{
    friend class CThemes;

    bool m_default;             ///< Is this button a default button?
    CThemeButtonType m_type;    ///< Button type - normal, thin, or combo box button
    CButtonKind m_kind;         ///< Button kind (for a stock image) or SP_UNDEFINED_BUTTON for user_defined image
    Fl_Image* m_image;          ///< Internal image pointer
    CIconSize m_iconSize;       ///< Icon size

    /**
     * Draws the dotted line. The line angle should be divided by 45 degrees w/o remains.
     * @param xs int, x-coordinate for the starting point
     * @param ys int, y-coordinate for the starting point
     * @param xe int, x-coordinate for the ending point
     * @param ye int, y-coordinate for the ending point
     */
    static void drawFocusLine(int xs, int ys, int xe, int ye);

protected:

    /**
     * Sets the button image to the selected kind buttonKind
     * @param buttonKind CButtonKind, the button kind
     * @param iconSize CIconSize, the size of the icon
     * @param label const char*, optional label
     */
    void image(CButtonKind buttonKind, CIconSize iconSize, String label = "");

    /**
     * Draws the button
     */
    void draw() override;

    /**
     * Draws the focus for the button.
     * @param usingTheme bool, to use theme's value for the focus corner radius. Otherwise, radius is 0.
     */
    void drawFocus(bool usingTheme);

    /**
     * Special handle() method
     */
    int handle(int event) override;

    /**
     * Preferred size for the button computes the optimal size for the button
     * @param w int&, button width
     * @param h int&, button height
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    bool preferredSize(int& w, int& h) override;

    /**
     * Constructor in SPTK style
     * @param kind CButtonKind, stock image id
     * @param layoutAlign CLayoutAlign, widget align in layout
     * @param is_small bool, true for the small button (small pixmap, no label)
     * @param label const char * label
     * @param tbt CThemeButtonType, the type of button
     */
    CBaseButton(CButtonKind kind, CLayoutAlign layoutAlign = CLayoutAlign::RIGHT, bool is_small = false,
                const char* label = nullptr, CThemeButtonType tbt = THM_BUTTON_NORMAL);

    /**
     * Constructor in SPTK style. The image should be assigned separately.
     * @param label const char *, label
     * @param layoutAlign CLayoutAlign, widget align in layout
     * @param tbt CThemeButtonType, the type of button
     */
    CBaseButton(const char* label = nullptr, CLayoutAlign layoutAlign = CLayoutAlign::RIGHT,
                CThemeButtonType tbt = THM_BUTTON_NORMAL);

#ifdef __COMPATIBILITY_MODE__
    /**
     * Constructor in FLTK style
     * @param kind CButtonKind, stock image id
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label const char *, label
     * @param tbt CThemeButtonType, the type of button
     */
    CBaseButton(CButtonKind kind,int x,int y,int w,const char *l=0,CThemeButtonType tbt=THM_BUTTON_NORMAL);
#endif

public:
    /**
     * Makes this button a default button
     *
     * Is used to define a button as default for the current
     * window of CDialog class. The default button is activated when an Enter button
     * is pressed inside CDialog on the widget and the widget doesn't process Enter
     * button.
     * @param defaultBtn bool makes button default or not
     */
    void defaultButton(bool defaultBtn);

    /**
     * Returns true if the button is a default button
     * @returns button type - default or not
     */
    bool defaultButton() const
    {
        return m_default;
    }

    /**
     * Returns widget class name (internal SPTK RTTI).
     */
    String className() const override
    {
        return "button";
    }

    /**
     * Sets the stock image to the button. Purely virtual.
     * @param bkind CButtonKind stock image id.
     * @param iconSize CIconSize, the size of the icon
     */
    virtual void buttonImage(CButtonKind bkind, CIconSize iconSize = CIconSize::IS_LARGE_ICON)
    {
        image(bkind, iconSize, label());
    }

    /**
     * Sets the image to the button. Purely virtual.
     * @param image Fl_Image * image pointer
     */
    virtual void buttonImage(Fl_Image* image)
    {
        m_image = image;
    }

    /**
     * Returns the button's image. Purely virtual.
     * @returns image pointer
     */
    virtual const Fl_Image* buttonImage() const
    {
        return m_image;
    }

    /**
     * Returns button label
     * @returns button label
     */
    const String& label() const override
    {
        return m_label;
    }

    /**
     * Sets button label
     * @param lbl const char *, new button label
     */
    void label(const String& lbl) override
    {
        m_label = lbl;
    }
};

/**
 * Button widget
 *
 * Button that displays an a regular Fl_Image *, or as a stock image of CButtonKind.
 */
class SP_EXPORT CButton
    : public CBaseButton
{
public:
#ifdef __COMPATIBILITY_MODE__
    /**
     * Constructor in FLTK style
     * @param kind CButtonKind stock image id
     * @param x int x-position
     * @param y int y-position
     * @param w int width
     * @param h int height
     * @param label const char * label
     */
    CButton(CButtonKind kind,int x,int y,int w,const char *label=0,CThemeButtonType tbt=THM_BUTTON_NORMAL)
            : CBaseButton(kind,x,y,w,label,tbt) {}
#endif

    /**
     * Constructor in SPTK style
     * @param kind CButtonKind stock image id
     * @param layoutAlign CLayoutAlign widget align in layout
     * @param label const char * label
     * @param tbt CThemeButtonType, the size type of the button
     */
    CButton(CButtonKind kind, CLayoutAlign layoutAlign = CLayoutAlign::RIGHT, const char* label = "",
            CThemeButtonType tbt = THM_BUTTON_NORMAL)
        : CBaseButton(kind, layoutAlign, false, label, tbt)
    {
    }

    /**
     * Default constructor in SPTK style
     * @param label const char * label
     * @param layoutAlign CLayoutAlign widget align in layout
     * @param tbt CThemeButtonType, the button type (size)
     */
    CButton(const char* label = "", CLayoutAlign layoutAlign = CLayoutAlign::RIGHT,
            CThemeButtonType tbt = THM_BUTTON_NORMAL)
        : CBaseButton(label, layoutAlign, tbt)
    {
    }

    /**
     * Creates a button based on the XML node information
     */
    static CLayoutClient* creator(const xdoc::SNode& node);

    /**
     * Loads layout client information from XML node
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node              the XML node
     * @param xmlMode           (ignored)
     */
    void load(const std::shared_ptr<xdoc::Node>& node, CLayoutXMLmode xmlMode) override
    {
        load(node);
    }

    /**
     * Loads the the widget from XML node
     *
     * The widget information may include widget attributes
     * and widget data
     * @param node xml::Node*, XML node
     */
    void load(const xdoc::SNode& node) override;

    /**
     * Saves the the widget to XML node
     *
     * The widget information may include widget attributes
     * and widget data
     * @param node              XML node
     * @param lm                (ignored)
     */
    void save(const std::shared_ptr<xdoc::Node>& node, CLayoutXMLmode lm = CLayoutXMLmode::LAYOUT) const override;
};

/**
 * Small button widget
 *
 * Displays a regular Fl_Image *, or as a stock image of CButtonKind.
 * The button label is ignored
 */
class SP_EXPORT CSmallButton
    : public CBaseButton
{
public:
#ifdef __COMPATIBILITY_MODE__
    /**
     * Constructor in FLTK style
     * @param kind CButtonKind stock image id
     * @param x int x-position
     * @param y int y-position
     * @param w int width
     * @param h int height
     * @param label const char * label
     */
    CSmallButton(CButtonKind kind,int x,int y,int w,const char *label=0)
            : CBaseButton(kind,x,y,w,label,THM_BUTTON_COMBO) {}
#endif

    /**
     * Default constructor in SPTK style
     * @param kind CButtonKind stock image id
     * @param layoutAlign CLayoutAlign widget align in layout
     * @param label const char *, label
     */
    CSmallButton(CButtonKind kind, CLayoutAlign layoutAlign = CLayoutAlign::RIGHT, const char* label = nullptr)
        : CBaseButton(kind, layoutAlign, true, label, THM_BUTTON_COMBO)
    {
    }
};
/**
 * @}
 */
}
