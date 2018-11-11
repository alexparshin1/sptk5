/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CColorSchema.h - description                           ║
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

#ifndef __CCOLORSCHEMA_H__
#define	__CCOLORSCHEMA_H__

#include <FL/Fl.H>
#include <map>

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

#define MAX_BUTTON_IMAGES 5

/**
 * @brief Stores and restores the color schema
 */
class CColorSchema {
    /**
     * Color map index
     */
    static Fl_Color             m_colorMapIndex[];

    /**
     * Color values
     */
    std::map<Fl_Color,unsigned> m_colorMap;

public:
    /**
     * @brief Store FLTK color schema to color map
     */
    void store();

    /**
     * @brief Restore FLTK color schema from color map
     */
    void restore();

    /**
     * @brief Number of colors in color schema
     */
    size_t size() const { return m_colorMap.size(); }
};

/**
 * @}
 */
}

#endif
