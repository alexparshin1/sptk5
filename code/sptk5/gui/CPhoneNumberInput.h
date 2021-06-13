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

#include <sptk5/gui/CInput.h>
#include <sptk5/gui/CControl.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * Phone number input widget
 *
 * Phone number input with two input boxes - area code and phone number.
 */
class SP_EXPORT CPhoneNumberInput
        : public CInput
{
    using inherited = class CInput;

    /**
     * Constructor initializer
     */
    void ctor_init();

public:

    /**
     * Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CPhoneNumberInput(const char* label = 0, int layoutSize = 10, CLayoutAlign layoutAlign = SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CPhoneNumberInput(int x,int y,int w,int h,const char *label=0);
#endif

    /**
     * Returns the control kind, SPTK-style RTTI
     * @see CControlKind for more information
     */
    CControlKind kind() const override
    {
        return DCV_PHONE;
    }

    /**
     * Returns the control class name, SPTK-style RTTI
     */
    String className() const override
    {
        return "phone_number";
    }

    /**
     * Universal data connection, returns data from control
     */
    Variant data() const override;

    /**
     * Universal data connection, sets data from control
     */
    void data(const Variant& v) override;

    /**
     * Returns true if the input data is valid
     */
    bool valid() const override;

    /**
     * Computes the optimal widget width
     * @param w int&, input - width offered by the program, output - width required by widget
     */
    void preferredWidth(int& w) const override;

    /**
     * Creates a widget based on the XML node information
     */
    static CLayoutClient* creator(xml::Node* node);
};

/**
 * @}
 */
}
