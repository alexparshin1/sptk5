/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CToolBar.cpp - description                             ║
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

#include <FL/fl_draw.H>
#include <sptk5/gui/CToolBar.h>

using namespace std;
using namespace sptk;

CToolBar::CToolBar(CLayoutAlign layoutAlignment)
    : CGroup("", 5, layoutAlignment)
{
    m_colapsed = false;
    layoutSpacing(4);
    box(FL_UP_BOX);
}

#ifdef __COMPATIBILITY_MODE__
CToolBar::CToolBar(int x,int y,int w,int h)
        : CGroup(x, y, w, h) {
    m_colapsed = false;
    layoutSpacing(3);
    box(FL_UP_BOX);
}
#endif

CLayoutClient* CToolBar::creator(xml::Node* node)
{
    auto widget = new CToolBar(SP_ALIGN_TOP);
    widget->load(node, LXM_LAYOUTDATA);
    return widget;
}

CButton* CToolBar::addButton(CButtonKind buttonKind, Fl_Callback_p cb, const char* label, CLayoutAlign buttonAlign)
{
    auto b = new CButton(buttonKind, buttonAlign, label);
    if (cb != nullptr)
        b->callback(cb);
    return b;
}

#define HIDE_BAR_WIDTH 8
#define HIDE_BAR_WIDTH_COLLAPSED 30

void CToolBar::resize(int xx, int yy, int ww, int hh)
{
    if (m_colapsed) {
        Fl_Widget::resize(xx, yy, ww, HIDE_BAR_WIDTH + Fl::box_dh(box()));
    } else {
        Fl_Widget::resize(xx, yy, ww, hh);
        xx += HIDE_BAR_WIDTH;
        ww -= HIDE_BAR_WIDTH;
        autoLayout(xx, yy, ww, hh, true);
    }
}

bool CToolBar::preferredSize(int& ww, int& hh)
{
    if (m_colapsed) {
        hh = HIDE_BAR_WIDTH + Fl::box_dh(box());
        int wm = HIDE_BAR_WIDTH_COLLAPSED + Fl::box_dw(box());
        if (ww < wm)
            ww = wm;
    } else {
        ww -= HIDE_BAR_WIDTH + 1;
        autoLayout(0, 0, ww, hh, false);
        ww += HIDE_BAR_WIDTH + 1;
    }
    return false;
}

int CToolBar::handle(int event)
{
    if (event == FL_PUSH) {
        int xzone = HIDE_BAR_WIDTH;
        if (m_colapsed)
            xzone = HIDE_BAR_WIDTH_COLLAPSED;

        if (Fl::event_x() <= x() + xzone) {
            m_colapsed = !m_colapsed;
            auto parentLayoutManager = dynamic_cast<CLayoutManager*>(parent());
            //CLayoutManager *parentLayoutManager = dynamic_cast<CLayoutManager *>(window());
            if (parentLayoutManager != nullptr)
                parentLayoutManager->relayout();
            else
                relayout();
            window()->redraw();
            return 1;
        }
    }
    return CGroup::handle(event);
}

void CToolBar::draw()
{
    int dx = Fl::box_dx(box());
    int dy = Fl::box_dy(box());
    int dh = Fl::box_dh(box());
    int xs = x() + dx + 1;
    int ys = y() + dy + 1;
    int hs = h() - dh - 2;
    if (m_colapsed) {
        draw_box();
        //fl_draw_box(FL_THIN_UP_BOX,xs,ys,HIDE_BAR_WIDTH_COLLAPSED,hs,color());
        //xs += 1;
        //ys += 1;
        int xe = xs + HIDE_BAR_WIDTH_COLLAPSED - 4;
        for (int xx = xs; xx < xe; xx += 3) {
            fl_draw_box(FL_THIN_UP_BOX, xx, ys, 2, 2, FL_WHITE);
            fl_draw_box(FL_THIN_UP_BOX, xx + 1, ys + 3, 2, 2, FL_WHITE);
        }
    } else {
        CGroup::draw();
        int ye = ys + hs - 2;
        for (int yy = ys; yy < ye; yy += 3) {
            fl_draw_box(FL_THIN_UP_BOX, xs, yy, 2, 2, FL_WHITE);
            fl_draw_box(FL_THIN_UP_BOX, xs + 3, yy + 1, 2, 2, FL_WHITE);
        }
    }
}
