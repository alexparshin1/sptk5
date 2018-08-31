/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CWindow.cpp - description                              ║
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

#include <sptk5/cxml>
#include <sptk5/gui/CWindow.h>

using namespace std;
using namespace sptk;

void CWindow::resize(int xx, int yy, int ww, int hh)
{
    Fl_Double_Window::resize(xx, yy, ww, hh);
    int www = ww, hhh = hh;
    // For a window, layout is always located at 0,0 coordinates
    autoLayout(0, 0, www, hhh, true);
    bool limited = false;
    if (www > Fl::w()) {
        www = Fl::w();
        limited = true;
    }
    if (hhh > Fl::h()) {
        hhh = Fl::h();
        limited = true;
    }
    if (xx + www > Fl::w()) {
        xx = Fl::w() - www;
        limited = true;
    }
    if (yy + hhh > Fl::h()) {
        yy = Fl::h() - hhh;
        limited = true;
    }

    if (limited || ww != www || hh != hhh)
        Fl_Double_Window::resize(xx, yy, www, hhh);

    if (m_shapeExtension)
        resizingShape(ww, hh);
}

bool CWindow::preferredSize(int& ww, int& hh)
{
    return autoLayout(0, 0, ww, hh, false);
}

void CWindow::hide()
{
    m_borderCleared = false;
    m_shapeChanged = true;
    Fl_Double_Window::hide();
}

void CWindow::draw()
{
    if (m_shapeExtension && m_shapeChanged) {
        resizingShape(w(), h());
        shapeApply();
    }

    int savex = x();
    x(0);
    int savey = y();
    y(0);

    Fl_Boxtype b = box();
    if (damage() & ~FL_DAMAGE_CHILD) { // redraw the entire thing:
        draw_box();
        paintBackground();
    }

    box(FL_NO_BOX);
    Fl_Double_Window::draw();
    box(b);

    y(savey);
    x(savex);
}

void CWindow::load(const xml::Node* node, CLayoutXMLmode xmlMode)
{
    CLayoutManager::loadLayout(node, xmlMode);
    loadPosition(node);
}

void CWindow::save(xml::Node* node, CLayoutXMLmode xmlMode) const
{
    CLayoutManager::saveLayout(node, xmlMode);
    savePosition(node);
}

void CWindow::loadPosition(const xml::Node* node)
{
    int hh = node->getAttribute("height");
    if (!hh)
        hh = node->getAttribute("h");
    int ww = node->getAttribute("width");
    if (!ww)
        ww = node->getAttribute("w");
    if (hh > 0 && ww > 0)
        resize(node->getAttribute("x", "0"), node->getAttribute("y", "0"), ww, hh);
}

void CWindow::savePosition(xml::Node* node) const
{
    node->setAttribute("x", x());
    node->setAttribute("y", y());
    node->setAttribute("width", w());
    node->setAttribute("height", h());
    node->setAttribute("label", label());
    node->name(className());
}

int CWindow::handle(int event)
{
    int rc = Fl_Window::handle(event);

    if (!m_shapeExtension)
        return rc;

    switch (event) {

        case FL_PUSH:
            m_pushedX = Fl::event_x();
            m_pushedY = Fl::event_y();
            m_resizingZone = mouseZone(m_pushedX, m_pushedY);
            break;

        case FL_DRAG:
            if (m_resizingZone) {
                int x, y;
                Fl::get_mouse(x, y);
                changeSize(x, y);
            } else {
                int new_x, new_y;
                new_x = Fl::event_x() + x() - m_pushedX;
                new_y = Fl::event_y() + y() - m_pushedY;
                position(new_x, new_y);
            }
            return 1;

        default:
            break;
    }

    if (shapeCursorHandle(event))
        return 1;

    return rc;
}
