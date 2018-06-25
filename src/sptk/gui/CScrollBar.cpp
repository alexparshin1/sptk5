/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CScrollBar.cpp - description                           ║
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

#if defined(WIN32)
#pragma warning (disable: 4786)
#endif

#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/fl_draw.H>
#include <sptk5/gui/CScrollBar.h>
#include <sptk5/gui/CThemes.h>

#include <cmath>

using namespace std;
using namespace sptk;

int CScrollBar::handle(int event)
{
    int area; // area of scrollbar
    int X = x(), Y = y(), W = w(), H = h();

    // adjust slider area to be inside the arrow buttons:
    if (horizontal()) {
        if (W >= 3 * H) {
            X += H;
            W -= 2 * H;
        }
    } else {
        if (H >= 3 * W) {
            Y += W;
            H -= 2 * W;
        }
    }

    // which widget part is highlighted?
    int relx;
    int ww;
    if (horizontal()) {
        relx = Fl::event_x() - X;
        ww = W;
    } else {
        relx = Fl::event_y() - Y;
        ww = H;
    }
    if (relx < 0)
        area = 1;
    else if (relx >= ww)
        area = 2;
    else {
        auto S = (int) std::round(slider_size() * ww);
        int T = (horizontal() ? H : W) / 2 + 1;
        if (S < T)
            S = T;
        double val =
                (maximum() - minimum()) ? (value() - minimum()) / (maximum() - minimum()) : 0.5;
        int sliderx;
        if (val >= 1.0)
            sliderx = ww - S;
        else if (val <= 0.0)
            sliderx = 0;
        else
            sliderx = (int) std::round(val * (ww - S));
        if (Fl::event_button() == FL_MIDDLE_MOUSE)
            area = 8;
        else if (relx < sliderx)
            area = 5;
        else if (relx >= sliderx + S)
            area = 6;
        else
            area = 8;
    }

    switch (event) {
        case FL_ENTER:
        case FL_LEAVE:
            break;
        case FL_RELEASE:
            m_pushed = 0;
            break;
        case FL_PUSH:
            if (m_pushed)
                break;
            if (area != 8)
                m_pushed = area;
            break;
        default:
            break;
    }
    return Fl_Scrollbar::handle(event);
}

void CScrollBar::draw()
{
    CThemes::drawScrollBar(x(), y(), w(), h(), (float) minimum(), (float) maximum(), (float) value(), slider_size(),
                           m_pushed, active_r() != 0);
}

bool CScrollBar::preferredSize(int& w, int& h)
{
    CThemes::sizeScrollBar(w, h);
    return false;
}
