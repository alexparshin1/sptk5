/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CFloatInput.h - description                            ║
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

#ifndef __CFLOATINPUT_H__
#define __CFLOATINPUT_H__

#include <sptk5/sptk.h>

#include <sptk5/gui/CInput.h>
#include <sptk5/gui/CControl.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Floating point value input widget.
 *
 * Allows to input only Floating point numbers, and you can also define the input value range for the valid().
 */
class SP_EXPORT CFloatInput : public CInput {
    typedef class CInput inherited;

    /**
     * @brief Minimum value (optional)
     */
    double   m_minValue;

    /**
     * @brief Maximum value (optional)
     */
    double   m_maxValue;

    /**
     * @brief Number of decimal places to show
     */
    int      m_decimalPlaces;

protected:

    /**
     * @brief Saves data to query
     */
    virtual void save(Query *);

    /**
     * @brief Returns true if the input data is valid
     */
    virtual bool valid() const;

public:

    /**
     * @brief Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CFloatInput(const char * label=0,int layoutSize=10,CLayoutAlign layoutAlign=SP_ALIGN_TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * @brief Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CFloatInput(int,int,int,int,const char * = 0);
#endif

    /**
     * @brief Sets limits for the value inside
     * @param limited bool, true if use the limits
     * @param min double, minimum value
     * @param max double, maximum value
     */
    void setLimits(bool limited,double min=0,double max=0);

    /**
     * @brief Returns the number of decimal places
     */
    int     decimalPlaces() const {
        return m_decimalPlaces;
    }

    /**
     * @brief Sets the number of decimal places to show
     */
    void    decimalPlaces(int dp) {
        m_decimalPlaces = dp;
    }

    /**
     * @brief Returns the control kind, SPTK-style RTTI
     * @see CControlKind for more information
     */
    virtual CControlKind kind() const {
        return DCV_FLOAT;
    }

    /**
     * @brief Returns the control class name, SPTK-style RTTI
     */
    virtual String className() const {
        return "float_input";
    }

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
