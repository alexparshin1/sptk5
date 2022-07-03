/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/gui/CControl.h>
#include <sptk5/gui/CInput.h>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * @brief Integer value input widget.
 *
 * Allows to input only integers, and you can also define the input value range for the valid().
 */
class SP_EXPORT CIntegerInput
    : public CInput
{
    using inherited = class CInput;

    /**
     * @brief Minimum value (optional)
     */
    int m_minValue;

    /**
     * @brief Maximum value (optional)
     */
    int m_maxValue;

protected:
    /**
     * @brief Saves data to query
     */
    void save(Query*) override;

    /**
     * @brief Returns true if the input data is valid
     */
    bool valid() const override;

public:
    /**
     * @brief Constructor in SPTK style
     * @param label const char *, label
     * @param layoutSize int, widget align in layout
     * @param layoutAlign CLayoutAlign, widget align in layout
     */
    CIntegerInput(const char* label = 0, int layoutSize = 10, CLayoutAlign layoutAlign = CLayoutAlign::TOP);

#ifdef __COMPATIBILITY_MODE__
    /**
     * @brief Constructor in FLTK style
     * @param x int, x-position
     * @param y int, y-position
     * @param w int, width
     * @param h int, height
     * @param label, const char * label
     */
    CIntegerInput(int x, int y, int w, int h, const char* label = 0);
#endif

    /**
     * @brief Sets limits for the value inside
     * @param limited bool, true if use the limits
     * @param min int, minimum value
     * @param max int, maximum value
     */
    void setLimits(bool limited, int min = 0, int max = 0);

    /**
     * @brief Returns the control kind, SPTK-style RTTI
     * @see CControlKind for more information
     */
    CControlKind kind() const override
    {
        return CControlKind::INTEGER;
    }

    /**
     * @brief Returns the control class name, SPTK-style RTTI
     */
    String className() const override
    {
        return "integer_input";
    }

    /**
     * @brief Creates a widget based on the XML node information
     */
    static CLayoutClient* creator(const xdoc::SNode& node);
};
/**
 * @}
 */
} // namespace sptk
