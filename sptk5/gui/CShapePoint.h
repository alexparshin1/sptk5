/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CShapePoint.h - description                            ║
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

#ifndef __CSHAPEPOINT_H__
#define __CSHAPEPOINT_H__

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

#ifdef _WIN32
typedef int32_t COORD;
#else
/**
 * @brief Coordinate type for shapes
 */
typedef int16_t COORD;
#endif

/**
 * @brief A point to define a shape
 */
class CShapePoint {
public:
    /**
     * X-coordinate
     */
    COORD       x;

    /**
     * Y-coordinate
     */
    COORD       y;

public:
    /**
     * @brief Default constructor
     * @param _x COORD, X-coordinate
     * @param _y COORD, Y-coordinate
     */
    CShapePoint(COORD _x=0,COORD _y=0) {
        x = _x;
        y = _y;
    }
    /**
     * @brief Copy constructor
     * @param p const CPoint&, point to copy from
     */
    CShapePoint(const CShapePoint& p) {
        x = p.x;
        y = p.y;
    }
};
/**
 * @}
 */
}
#endif
