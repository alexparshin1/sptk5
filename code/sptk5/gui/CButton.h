/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CButton.h - description                                ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __CBUTTON_H__
#define __CBUTTON_H__

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
 * @brief Button widget kind
 *
 * The button kind defines a picture and a default label for the button.
 */
enum CButtonKind
{
    SP_UNDEFINED_BUTTON = 0,        ///< Default button without image, or with user-defined image
    SP_OK_BUTTON = 1,               ///< 'Ok' button
    SP_CANCEL_BUTTON = 2,           ///< 'Cancel' button
    SP_NO_BUTTON = 4,               ///< 'No' button
    SP_ADD_BUTTON = 8,              ///< 'Add' button
    SP_DELETE_BUTTON = 0x10,        ///< 'Delete' button
    SP_EDIT_BUTTON = 0x20,          ///< 'Edit' button
    SP_BROWSE_BUTTON = 0x40,        ///< 'Browse' button
    SP_REFRESH_BUTTON = 0x80,       ///< 'Refresh' button
    SP_CALENDAR_BUTTON = 0x100,     ///< 'Calendar' button
    SP_OPEN_BUTTON = 0x200,         ///< 'Open' button
    SP_PRINT_BUTTON = 0x400,        ///< 'Print' button
    SP_SAVE_BUTTON = 0x800,         ///< 'Save' button
    SP_SAVE_AS_BUTTON = 0x1000,     ///< 'Save As' button
    SP_COPY_BUTTON = 0x2000,        ///< 'Copy' button
    SP_LEFT_BUTTON = 0x4000,        ///< 'Left' button
    SP_NEW_BUTTON = 0x8000,         ///< 'New' button
    SP_NEXT_BUTTON = 0x10000,       ///< 'Next' button
    SP_PRINTER_BUTTON = 0x20000,    ///< 'Printer' button
    SP_PRIOR_BUTTON = 0x40000,      ///< 'Prior' button
    SP_RIGHT_BUTTON = 0x80000,      ///< 'Right' button
    SP_SEARCH_BUTTON = 0x100000,    ///< 'Left' button
    SP_SEND_BUTTON = 0x200000,      ///< 'Send' button
    SP_STEPLEFT_BUTTON = 0x400000,  ///< 'Step Left' button
    SP_STEPRIGHT_BUTTON = 0x800000, ///< 'Step Right' button
    SP_VIEW_BUTTON = 0x1000000,     ///< 'View' button
    SP_HOME_BUTTON = 0x2000000,     ///< 'Home' button
    SP_CONFIGURE_BUTTON = 0x4000000,///< 'Configure' button
    SP_EXEC_BUTTON = 0x8000000,     ///< Execute' button
    SP_STOP_BUTTON = 0x10000000,    ///< 'Stop' button
    SP_EXIT_BUTTON = 0x20000000,    ///< 'Exit' button
    SP_HELP_BUTTON = 0x40000000,    ///< 'Help' button
    SP_MAX_BUTTON = 0x40000001      ///< Max button id
};

/**
 * @brief Base button widget
 *
 * Base class for CButton and CSmallButton, uses Fl_Image * or a stock image of CButtonKind.
 */
class SP_EXPORT CBaseButton : public Fl_Button, public CLayoutClient
{
    friend class CThemes;

    /**
     * Is this button a default button?
     */
    bool m_default;

    /**
     * Button type - normal, thin, or combo box button
     */
    CThemeButtonType m_type;


    /**
     * @brief Draws the dotted line. The line angle should be divided by 45 degrees w/o remains.
     * @param xs int, x-coordinate for the starting point
     * @param ys int, y-coordinate for the starting point
     * @param xe int, x-coordinate for the ending point
     * @param ye int, y-coordinate for the ending point
     */
    void drawFocusLine(int xs, int ys, int xe, int ye);

    /**
     * Button kind (for a stock image) or SP_UNDEFINED_BUTTON for user_defined image
     */
    CButtonKind m_kind;

protected:

    /**
     * Internal image pointer
     */
    Fl_Image* m_image;

    /**
     * Icon size
     */
    CIconSize m_iconSize;


    /**
     * @brief Sets the button image to the selected kind buttonKind
     * @param buttonKind CButtonKind, the button kind
     * @param iconSize CIconSize, the size of the icon
     * @param label const char*, optional label
     */
    void image(CButtonKind buttonKind, CIconSize iconSize, String label = "");

    /**
     * @brief Draws the button
     */
    void draw() override;

    /**
     * @brief Draws the focus for the button.
     * @param usingTheme bool, to use theme's value for the focus corner radius. Otherwise, radius is 0.
     */
    void drawFocus(bool usingTheme);

    /**
     * Special handle() method
     */
    int handle(int event) override;

    /**
     * @brief Preferred size for the button computes the optimal size for the button
     * @param w int&, button width
     * @param h int&, button height
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    bool preferredSize(int& w, int& h) override;

    /**
     * @brief Constructor in SPTK style
     * @param kind CButtonKind, stock image id
     * @param layoutAlign CLayoutAlign, widget align in layout
     * @param is_small bool, true for the small button (small pixmap, no label)
     * @param label const char * label
     * @param tbt CThemeButtonType, the type of button
     */
    CBaseButton(CButtonKind kind, CLayoutAlign layoutAlign = SP_ALIGN_RIGHT, bool is_small = false,
                const char* label = 0, CThemeButtonType tbt = THM_BUTTON_NORMAL);

    /**
     * @brief Constructor in SPTK style. The image should be assigned separately.
     * @param label const char *, label
     * @param layoutAlign CLayoutAlign, widget align in layout
     * @param tbt CThemeButtonType, the type of button
     */
    CBaseButton(const char* label = 0, CLayoutAlign layoutAlign = SP_ALIGN_RIGHT,
                CThemeButtonType tbt = THM_BUTTON_NORMAL);

#ifdef __COMPATIBILITY_MODE__
    /**
     * @brief Constructor in FLTK style
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
     * @brief Makes this button a default button
     *
     * Is used to define a button as default for the current
     * window of CDialog class. The default button is activated when an Enter button
     * is pressed inside CDialog on the widget and the widget doesn't process Enter
     * button.
     * @param defaultBtn bool makes button default or not
     */
    void defaultButton(bool defaultBtn);

    /**
     * @brief Returns true if the button is a default button
     * @returns button type - default or not
     */
    bool defaultButton() const
    {
        return m_default;
    }

    /**
     * @brief Returns widget class name (internal SPTK RTTI).
     */
    String className() const override
    {
        return "button";
    }

    /**
     * @brief Sets the stock image to the button. Purely virtual.
     * @param bkind CButtonKind stock image id.
     * @param iconSize CIconSize, the size of the icon
     */
    virtual void buttonImage(CButtonKind bkind, CIconSize iconSize) = 0;

    /**
     * @brief Sets the image to the button. Purely virtual.
     * @param image Fl_Image * image pointer
     */
    virtual void buttonImage(Fl_Image* image) = 0;

    /**
     * @brief Returns the button's image. Purely virtual.
     * @returns image pointer
     */
    virtual Fl_Image* buttonImage() = 0;

    /**
     * @brief Returns button label
     * @returns button label
     */
    const String& label() const override
    {
        return m_label;
    }

    /**
     * @brief Sets button label
     * @param lbl const char *, new button label
     */
    void label(const String& lbl) override
    {
        m_label = lbl;
    }
};

/**
 * @brief Button widget
 *
 * Button that displays an a regular Fl_Image *, or as a stock image of CButtonKind.
 */
class SP_EXPORT CButton : public CBaseButton
{
public:
#ifdef __COMPATIBILITY_MODE__
    /**
     * @brief Constructor in FLTK style
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
     * @brief Constructor in SPTK style
     * @param kind CButtonKind stock image id
     * @param layoutAlign CLayoutAlign widget align in layout
     * @param label const char * label
     * @param tbt CThemeButtonType, the size type of the button
     */
    CButton(CButtonKind kind, CLayoutAlign layoutAlign = SP_ALIGN_RIGHT, const char* label = 0,
            CThemeButtonType tbt = THM_BUTTON_NORMAL)
            : CBaseButton(kind, layoutAlign, false, label, tbt)
    {}

    /**
     * @brief Default constructor in SPTK style
     * @param label const char * label
     * @param layoutAlign CLayoutAlign widget align in layout
     * @param tbt CThemeButtonType, the button type (size)
     */
    CButton(const char* label = 0, CLayoutAlign layoutAlign = SP_ALIGN_RIGHT, CThemeButtonType tbt = THM_BUTTON_NORMAL)
            : CBaseButton(label, layoutAlign, tbt)
    {}

    /**
     * @brief Sets the image to the button.
     * @param image const Fl_Image * image pointer
     */
    virtual void buttonImage(Fl_Image* image)
    {
        m_image = image;
    }

    /**
     * @brief Sets the stock image to the button.
     * @param bkind CButtonKind stock image id.
     * @param iconSize CIconSize, the size of the icon
     */
    virtual void buttonImage(CButtonKind bkind, CIconSize iconSize = IS_LARGE_ICON)
    {
        image(bkind, iconSize, label());
    }

    /**
     * @brief Returns the button's image.
     * @returns image pointer
     */
    virtual Fl_Image* buttonImage()
    {
        return m_image;
    }

    /**
     * @brief Creates a button based on the XML node information
     */
    static CLayoutClient* creator(xml::Node* node);

    /**
     * @brief Loads the the widget from XML node
     *
     * The widget information may include widget attributes
     * and widget data
     * @param node xml::Node*, XML node
     */
    virtual void load(const xml::Node* node);

    /**
     * @brief Saves the the widget to XML node
     *
     * The widget information may include widget attributes
     * and widget data
     * @param node xml::Node*, XML node
     */
    virtual void save(xml::Node* node) const;
};

/**
 * @brief Small button widget
 *
 * Displays a regular Fl_Image *, or as a stock image of CButtonKind.
 * The button label is ignored
 */
class SP_EXPORT CSmallButton : public CBaseButton
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
    CSmallButton(CButtonKind kind, CLayoutAlign layoutAlign = SP_ALIGN_RIGHT, const char* label = 0)
            : CBaseButton(kind, layoutAlign, true, label, THM_BUTTON_COMBO)
    {}

    /**
     * Sets the image to the button.
     * @param image const Fl_Image *, image pointer
     */
    virtual void buttonImage(Fl_Image* image)
    {
        m_image = image;
    }

    /**
     * Sets the stock image to the button.
     * @param bkind CButtonKind, stock image id.
     * @param iconSize CIconSize, the size of the icon
     */
    virtual void buttonImage(CButtonKind bkind, CIconSize iconSize = IS_SMALL_ICON)
    {
        image(bkind, iconSize);
    }

    /**
     * Returns the button's image.
     * @returns image pointer
     */
    virtual Fl_Image* buttonImage()
    {
        return m_image;
    }
};
/**
 * @}
 */
}
#endif
