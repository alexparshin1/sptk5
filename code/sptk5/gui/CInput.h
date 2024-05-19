/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <FL/Fl_Input.H>

#include <sptk5/gui/CControl.h>
#include <sptk5/gui/CLayoutClient.h>
#include <sptk5/sptk.h>

#include <string>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * Base input widget
 *
 * Extends the standard Fl_Input with the extra
 * input types for the date, time, and phone input masks. It also
 * supports the optional maximum input length.
 * In is used mainly as the internal widget in different CControl-derived
 * widgets.
 */
class SP_EXPORT CInput_
    : public Fl_Input
    , public CLayoutClient
{

    /**
     * Maximum input length
     */
    int m_maxLength;

    /**
     * Full input mask
     */
    std::string m_mask;

    /**
     * Actual input mask
     */
    std::string m_inputMask;

    /**
     * Background (static) characters in mask
     */
    std::string m_backgroundMask;


    /**
     * Internal check for the character for matching the mask
     */
    bool checkCharacterAtPos(int pos, char key);

    /**
     * Internal check for the character for matching the mask
     */
    bool checkCharacter(int pos, char& key);

protected:
    /**
     * Conversion of the outside input value into internal masked value
     * using mask (if defined)
     */
    void maskValue();

public:
    /**
     * Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CInput_(const char* label = 0, int layoutSize = 20, CLayoutAlign layoutAlign = CLayoutAlign::TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CInput_(int x, int y, int w, int h, const char* label = 0);
#endif

    /**
     * CInput_-specific version of FLTK handle() function
     * @param event int, FLTK event
     * @returns true if the event was processed
     */
    int handle(int event) override;

    /**
     * Returns the font size for the input text
     */
    uchar textSize() const
    {
        return (uchar) textsize();
    }

    /**
     * Sets the font size for the input text
     */
    void textSize(uchar sz)
    {
        textsize(sz);
    }

    /**
     * Returns the font type for the input text
     */
    Fl_Font textFont() const
    {
        return textfont();
    }

    /**
     * Sets the font type for the input text
     */
    void textFont(Fl_Font fnt)
    {
        textfont(fnt);
    }

    /**
     * Returns the maximum length for the input text
     */
    int maxLength() const
    {
        return m_maxLength;
    }

    /**
     * Sets the maximum length for the input text
     */
    void maxLength(int ml)
    {
        m_maxLength = ml;
    }

    /**
     * Computes the optimal widgets size
     * @param w int&, input - width offered by the program, output - width required by widget
     * @param h int&, input - height offered by the program, output - height required by widget
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    bool preferredSize(int& w, int& h) override;

    /**
     * Sets the optional mask to the input widget. Passing an empty string as the maskString clears the mask.
     * @param maskString const char *, the mask string.
     */
    void mask(const char* maskString);
};

/**
 * Text input widget.
 *
 * Implements the simple single-line input box.
 * While it can be used by itself, it's also a base class for
 * several other SPTK widgets. As CControl descendant it's a
 * layout client.
 */
class SP_EXPORT CInput
    : public CControl
{

    /**
     * Constructor initializer
     */
    void ctor_init(bool autoCreate);

protected:
    /**
     * Loads data from query
     */
    void load(Query*) override;

    /**
     * Saves data to query
     */
    void save(Query*) override;

    /**
     * Loads control data from XML
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node xml::Node*, the XML node
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be stored
     */
    void load(const xdoc::SNode& node, CLayoutXMLmode xmlMode) override
    {
        CControl::load(node, xmlMode);
    }

    /**
     * Saves control data to XML
     *
     * Layout information may also include widget size and position,
     * as well as visible() and active() states
     * @param node xml::Node*, the XML node
     * @param xmlMode CLayoutXMLmode, the mode defining how the layout and/or data should be stored
     */
    void save(const xdoc::SNode& node, CLayoutXMLmode xmlMode) const override
    {
        CControl::save(node, xmlMode);
    }

    /**
     * Returns true if the input data is valid
     */
    bool valid() const override
    {
        return true;
    }

    /**
     * Special internal constructor for derived classes
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     * @param autoCreate bool, if true CControl will create CInput_ widget and make it the main internal widget.
     */
    CInput(const char* label, int layoutSize, CLayoutAlign layoutAlign, bool autoCreate);

public:
    /**
     * Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CInput(const char* label = 0, int layoutSize = 10, CLayoutAlign layoutAlign = CLayoutAlign::TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CInput(int, int, int, int, const char* = 0, bool autoCreate = true);
#endif

    /**
     * Destructor
     */
    ~CInput() override;

    /**
     * Returns input widget input type
     */
    int controlType() const;

    /**
     * Sets input widget input type
     */
    void controlType(int type);

    /**
     * Universal data connection, returns data from control
     */
    Variant data() const override;

    /**
     * Universal data connection, sets data from control
     */
    void data(const Variant& v) override;

    /**
     * Returns the control kind, SPTK-style RTTI
     * @see CControlKind for more information
     */
    CControlKind kind() const override
    {
        return CControlKind::STRING;
    }

    /**
     * Returns the control class name, SPTK-style RTTI
     */
    String className() const override
    {
        return "input";
    }

    /**
     * Returns the input text font type
     */
    Fl_Font textFont() const override;

    /**
     * Sets the input text font type
     */
    void textFont(Fl_Font f) override;

    /**
     * Returns the input text font size
     */
    uchar textSize() const override;

    /**
     * Sets the input text font size
     */
    void textSize(uchar s) override;

    /**
     * Returns the input text maximum length, 0 - unlimited
     */
    int maxLength() const override;

    /**
     * Sets the input text maximum length, 0 - unlimited
     */
    void maxLength(int) override;

    /**
     * Computes the optimal widget width
     * @param w int&, input - width offered by the program, output - width required by widget
     */
    virtual void preferredWidth(int& w) const;

    /**
     * Computes the optimal widget height
     * @param h int&, input - height offered by the program, output - height required by widget
     */
    virtual void preferredHeight(int& h) const;

    /**
     * Computes the optimal widget size
     * @param w int&, input - width offered by the program, output - width required by widget
     * @param h int&, input - height offered by the program, output - height required by widget
     * @returns true if the size is stable (doesn't depend on input sizes)
     */
    bool preferredSize(int& w, int& h) override;

    /**
     * Creates a widget based on the XML node information
     */
    static CLayoutClient* creator(const xdoc::SNode& node);
};
/**
 * @}
 */
} // namespace sptk
