/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CWindowShape.cpp - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <sptk5/sptk.h>

#include <sptk5/gui/CWindow.h>
#include <FL/fl_draw.H>

using namespace std;
using namespace sptk;

#ifndef _WIN32

#include <X11/extensions/shape.h>

#endif

#define TOP_EDGE    1
#define BOTTOM_EDGE 2
#define LEFT_EDGE   4
#define RIGHT_EDGE  8

CWindowShape::CWindowShape(CWindow* window)
{
    m_window = window;
    m_shapeChanged = true;
    m_lastW = m_lastH = 0;
    m_resizingZone = 0;
    m_borderWidth = 6;
    m_borderCleared = false;
    m_shapeExtension = false;
    m_pushedX = 0;
    m_pushedY = 0;
}

void CWindowShape::initShapeExtension()
{
    m_shapeExtension = true;
#ifdef _WIN32
    if (m_window)
        m_window->clear_border();
#endif
}

void CWindowShape::shapeApply()
{
    if (!m_shapeExtension)
        return;
    if (!m_shapePoints.empty()) {
        CShapePoint* array = &m_shapePoints[0];
#ifdef _WIN32
        HRGN region = CreatePolygonRgn((CONST POINT*)array, (int) m_shapePoints.size(), WINDING);
        SetWindowRgn(fl_xid(m_window), region, TRUE);
        if (!m_borderCleared) {
            //ShowWindow(fl_xid(m_window),SW_HIDE);
            LONG style = GetWindowLong(fl_xid(m_window), GWL_EXSTYLE);
            style |= WS_EX_APPWINDOW;
            SetWindowLong(fl_xid(m_window), GWL_EXSTYLE, style);
            m_borderCleared = true;
            //ShowWindow(fl_xid(m_window),SW_SHOW);
        }
#else
        Region region = XPolygonRegion((XPoint*) array, (int) m_shapePoints.size(), WindingRule);
        //XUnmapWindow(fl_display, fl_xid(m_window));
        XShapeCombineRegion(fl_display, fl_xid(m_window), ShapeBounding, 0, 0, region, ShapeSet);

        if (!m_borderCleared) {
            int xpos = m_window->x();
            int ypos = m_window->y();
            // Defining window with no caption
            long prop[5] = {3, 1, 0, 0, 0};
            Atom motif_wm_hints = XInternAtom(fl_display, "_MOTIF_WM_HINTS", 0);
            XChangeProperty(fl_display, fl_xid(m_window), motif_wm_hints, motif_wm_hints, 32,
                            PropModeReplace, (unsigned char*) prop, 5);
            m_borderCleared = true;
            //XDeleteProperty (fl_display, fl_xid(m_window),net_wm_state);
            XMoveWindow(fl_display, fl_xid(m_window), xpos, ypos);
        }

        XMapWindow(fl_display, fl_xid(m_window));
        if (region != None)
            XDestroyRegion(region);
#endif

    }
    m_shapeChanged = false;
}

int CWindowShape::mouseZone(int mouseX, int mouseY) const
{
    int zone = 0;
    int zoneWidth = m_borderWidth * 2;

    for (unsigned i = 0; i < 2; i++) {
        if (mouseX < zoneWidth) {
            zone = LEFT_EDGE;
        } else if (mouseX > m_window->w() - zoneWidth)
            zone = RIGHT_EDGE;
        if (mouseY < m_borderWidth)
            zone |= TOP_EDGE;
        else if (mouseY > m_window->h() - zoneWidth)
            zone |= BOTTOM_EDGE;
        if (zone & (LEFT_EDGE | RIGHT_EDGE) && zone & (TOP_EDGE | BOTTOM_EDGE))
            return zone; // Mouse is on the corner
        zoneWidth = m_borderWidth;
    }
    return zone;
}

void CWindowShape::changeSize(int mouseX, int mouseY)
{
    if (!m_shapeExtension)
        return;
    if (!m_resizingZone)
        return;
    int oldX = m_window->x(), oldY = m_window->y(), oldW = m_window->w(), oldH = m_window->h();
    int newX = oldX, newY = oldY, newW = oldW, newH = oldH;

    if (m_resizingZone & TOP_EDGE) {
        if (oldY != mouseY) {
            newH = oldY + oldH - mouseY;
            newY = mouseY;
        }
    } else if (m_resizingZone & BOTTOM_EDGE) {
        if (oldY + oldH != mouseY)
            newH = mouseY - oldY;
    }

    if (m_resizingZone & LEFT_EDGE) {
        if (oldX != mouseX) {
            newX = mouseX;
            newW = oldX + oldW - mouseX;
        }
    } else if (m_resizingZone & RIGHT_EDGE) {
        if (oldX + oldW != mouseX)
            newW = mouseX - oldX;
    }

    if (newX != oldX || newY != oldY)
        m_window->resize(newX, newY, newW, newH);
    else if (newW != oldW || newH != oldH)
        m_window->size(newW, newH);
}

int CWindowShape::shapeCursorHandle(int event)
{
    if (!m_shapeExtension)
        return 0;
    switch (event) {
        case FL_ENTER:
        case FL_LEAVE:
            return 1;
        case FL_MOVE:
            switch (mouseZone(Fl::event_x(), Fl::event_y())) {
                case LEFT_EDGE | TOP_EDGE:
                case RIGHT_EDGE | BOTTOM_EDGE:
                    m_window->cursor(FL_CURSOR_NWSE);
                    return 1;
                case TOP_EDGE:
                case BOTTOM_EDGE:
                    m_window->cursor(FL_CURSOR_NS);
                    return 1;
                case LEFT_EDGE:
                case RIGHT_EDGE:
                    m_window->cursor(FL_CURSOR_WE);
                    return 1;
                case LEFT_EDGE | BOTTOM_EDGE:
                case RIGHT_EDGE | TOP_EDGE:
                    m_window->cursor(FL_CURSOR_NESW);
                    return 1;
                default:
                    break;
            }
            m_window->cursor(FL_CURSOR_DEFAULT);
            return 1;
        default:
            break;
    }
    return 0;
}

