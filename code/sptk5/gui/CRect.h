/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CRect.h - description                                  ║
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

#ifndef __CRECT_H__
#define __CRECT_H__

namespace sptk {

/**
 * @addtogroup gui GUI Classes
 * @{
 */

/**
 * General class to work with rectangles
 */
class CRect {
    /**
     * rectangle X-coordinate
     */
    int m_x;

    /**
     * rectangle Y-coordinate
     */
    int   m_y;

    /**
     * rectangle width
     */
    int   m_w;

    /**
     * rectangle height
     */
    int   m_h;

public:
    /**
     * Default constructor
     * @param x int, rectangle X-coordinate
     * @param y int, rectangle Y-coordinate
     * @param w int, rectangle width
     * @param h int, rectangle height
     */
    CRect(int x=0,int y=0,int w=0,int h=0) {
        m_x = x;
        m_y = y;
        m_w = w;
        m_h = h;
    }

    /**
     * returns rectangle X-coordinate
     */
    int x() const {
        return m_x;
    }

    /**
     * returns rectangle Y-coordinate
     */
    int y() const {
        return m_y;
    }

    /**
     * returns rectangle width
     */
    int w() const {
        return m_w;
    }

    /**
     * returns rectangle height
     */
    int h() const {
        return m_h;
    }

    /**
     * sets rectangle X and Y coordinates
     * @param x int, rectangle X-coordinate
     * @param y int, rectangle Y-coordinate
     */
    void move(int x,int y) {
        m_x = x;
        m_y = y;
    }

    /**
     * sets rectangle's width and height
     * @param w int, rectangle width
     * @param h int, rectangle height
     */
    void size(int w,int h) {
        m_w = w;
        m_h = h;
    }

    /**
     * shifts rectangle X and Y coordinates
     * @param dx int, rectangle X-coordinate shift
     * @param dy int, rectangle Y-coordinate shift
     */
    void shift(int dx,int dy) {
        m_x += dx;
        m_y += dy;
    }

    /**
     * shrinks rectangle width and height
     * @param dw int, rectangle width change
     * @param dh int, rectangle height change
     */
    void shrink(int dw,int dh) {
        m_w -= dw;
        m_h -= dh;
    }

    /**
     * expands rectangle width and height
     * @param dw int, rectangle width change
     * @param dh int, rectangle height change
     */
    void expand(int dw,int dh) {
        m_w += dw;
        m_h += dh;
    }
}
;
/**
 * @}
 */
}

#endif
