/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CGroup.cpp - description                               ║
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

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <sptk5/gui/CControl.h>
#include <sptk5/gui/CGroup.h>

using namespace std;
using namespace sptk;

void CGroup::ctor_init(const char* alabel)
{
    if (alabel == nullptr)
        alabel = "";
    m_drawClipped = true;
    label(alabel);
}

CGroup::CGroup(const char* alabel, int layoutSize, CLayoutAlign layoutAlign)
        : Fl_Group(0, 0, layoutSize, layoutSize), CLayoutManager(this, layoutSize, layoutAlign)
{
    ctor_init(alabel);
}

#ifdef __COMPATIBILITY_MODE__
CGroup::CGroup(int x,int y,int w,int h,const char *alabel)
        : Fl_Group(x,y,w,h), CLayoutManager(this,w,SP_ALIGN_NONE) {
    ctor_init(alabel);
}
#endif

CLayoutClient* CGroup::creator(xml::Node* node)
{
    auto widget = new CGroup("", 10, SP_ALIGN_TOP);
    widget->loadLayout(node, LXM_LAYOUTDATA);
    return widget;
}

void CGroup::resize(int xx, int yy, int ww, int hh)
{
    Fl_Widget::resize(xx, yy, ww, hh);

    //int offsetX = frameWidth();
    //int offsetY = offsetX;

    const char* lbl = m_label.c_str();
    if ((align() & FL_ALIGN_INSIDE) != 0 && lbl != nullptr && lbl[0] != 0) {
        int tw = ww - 8, th = 0;
        fl_font(labelfont(), labelsize());
        fl_measure(lbl, tw, th);
        //offsetY = th;
    }

    autoLayout(xx, yy, ww, hh, true);
}

bool CGroup::preferredSize(int& ww, int& hh)
{
    int offsetX = frameWidth();
    int offsetY = offsetX;

    const char* lbl = m_label.c_str();
    if ((align() & FL_ALIGN_INSIDE) != 0 && lbl != nullptr && lbl[0] != 0) {
        int tw = ww - 8, th = 0;
        fl_font(labelfont(), labelsize());
        fl_measure(lbl, tw, th);
        offsetY = th;
    }

    return autoLayout(offsetX, offsetY, ww, hh, false);
}

void CGroup::draw()
{
    string lbl = label();
    label("");

    Fl_Boxtype b = box();

    if (damage() & ~FL_DAMAGE_CHILD) { // redraw the entire thing:
        if (m_frame == nullptr)
            fl_draw_box(box(), x(), y(), w(), h(), color());
        paintBackground();
    }

    box(FL_NO_BOX);

    if (m_drawClipped)
        fl_push_clip(x() + Fl::box_dx(b), y() + Fl::box_dy(b), w() - Fl::box_dw(b), h() + Fl::box_dh(b));

    Fl_Group::draw();

    if (m_drawClipped)
        fl_pop_clip();

    label(lbl.c_str());

    int tw = w() - 8, th = 0;
    fl_font(labelfont(), labelsize());
    fl_measure(m_label.c_str(), tw, th);

    if ((align() & FL_ALIGN_INSIDE) && lbl[0]) {
        int xt = x() + 8;
        switch (align() & (FL_ALIGN_LEFT | FL_ALIGN_RIGHT | FL_ALIGN_CENTER)) {
            case FL_ALIGN_LEFT:
                break;
            case FL_ALIGN_RIGHT:
                xt = x() + w() - 8 - (tw + 4);
                break;
            case FL_ALIGN_CENTER:
                xt = x() + (w() - (tw + 4)) / 2;
                break;
            default:
                break;
        }
        fl_draw_box(FL_FLAT_BOX, xt, y(), tw + 4, th + 2, color());
        fl_color(labelcolor());
        fl_draw(m_label.c_str(), xt, y() + 2, tw + 4, th, FL_ALIGN_CENTER);
    } else {
        draw_label();
    }
    box(b);
}
