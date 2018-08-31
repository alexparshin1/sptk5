/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CPhoneNumberInput.h - description                      ║
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

#ifndef __CPHONENUMBERINPUT_H__
#define __CPHONENUMBERINPUT_H__

#include <sptk5/sptk.h>

#include <sptk5/gui/CInput.h>
#include <sptk5/gui/CControl.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Phone number input widget
 *
 * Phone number input with two input boxes - area code and phone number.
 */
class SP_EXPORT CPhoneNumberInput : public CInput {
    typedef class CInput inherited;

    /**
     * @brief Constructor initializer
     */
    void ctor_init();

public:

    /**
     * @brief Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CPhoneNumberInput(const char * label=0,int layoutSize=10,CLayoutAlign layoutAlign=SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * @brief Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CPhoneNumberInput(int x,int y,int w,int h,const char *label=0);
#endif

    /**
     * @brief Returns the control kind, SPTK-style RTTI
     * @see CControlKind for more information
     */
    virtual CControlKind kind() const {
        return DCV_PHONE;
    }

    /**
     * @brief Returns the control class name, SPTK-style RTTI
     */
    virtual String className() const {
        return "phone_number";
    }

    /**
     * @brief Universal data connection, returns data from control
     */
    virtual Variant data() const;

    /**
     * @brief Universal data connection, sets data from control
     */
    virtual void     data(const Variant v);

    /**
     * @brief Returns true if the input data is valid
     */
    virtual bool valid() const;

    /**
     * @brief Computes the optimal widget width
     * @param w int&, input - width offered by the program, output - width required by widget
     */
    virtual void     preferredWidth(int& w) const;

    /**
     * @brief Creates a widget based on the XML node information
     */
    static CLayoutClient* creator(xml::Node *node);
};

/**
 * @}
 */
}
#endif
