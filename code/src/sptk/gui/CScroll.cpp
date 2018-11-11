/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CScroll.cpp - description                              ║
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

#include <sptk5/gui/CGroup.h>
#include <sptk5/gui/CControl.h>
#include <sptk5/gui/CScroll.h>

using namespace std;
using namespace sptk;

#define SLIDER_WIDTH 16

void CScroll::ctor_init()
{
    box(FL_FLAT_BOX);
    type(BOTH);
    m_xposition = 0;
    m_yposition = 0;
    m_oldx = 0;
    m_oldy = 0;
    hscrollbar.type(FL_HORIZONTAL);
    hscrollbar.callback(hscrollbar_cb);
    scrollbar.callback(scrollbar_cb);
}

CScroll::CScroll(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        : Fl_Group(0, 0, layoutSize, layoutSize, label),
          CLayoutManager(this, layoutSize, layoutAlignment),
          scrollbar(layoutSize - SLIDER_WIDTH, 0, SLIDER_WIDTH, layoutSize - SLIDER_WIDTH),
          hscrollbar(0, layoutSize - SLIDER_WIDTH, layoutSize - SLIDER_WIDTH, SLIDER_WIDTH)
{
    ctor_init();
}

#ifdef __COMPATIBILITY_MODE__
CScroll::CScroll(int x,int y,int w,int h,const char *l)
        : Fl_Group(x,y,w,h,l),
        CLayoutManager(this,w,SP_ALIGN_NONE),
        scrollbar(x+w-SLIDER_WIDTH,y,SLIDER_WIDTH,h-SLIDER_WIDTH),
hscrollbar(x,y+h-SLIDER_WIDTH,w-SLIDER_WIDTH,SLIDER_WIDTH) {
    ctor_init();
}
#endif

CLayoutClient* CScroll::creator(xml::Node* node)
{
    auto widget = new CScroll("", 10, SP_ALIGN_TOP);
    widget->loadLayout(node, LXM_LAYOUTDATA);
    return widget;
}

bool CScroll::preferredSize(int& ww, int& hh)
{
    int minh = 10;
    int minw = 10;
    if (type()) {
        int sbw = 0;
        int sbh = 0;
        scrollbar.preferredSize(sbw, minh);
        scrollbar.preferredSize(minw, sbh);
    }
    if (hh < minh)
        hh = minh;
    if (ww < minw)
        ww = minw;
    return false;
}

void CScroll::clear()
{
    for (int i = children() - 1; i >= 0; i--) {
        Fl_Widget* o = child(i);
        if (o != &hscrollbar && o != &scrollbar)
            delete o;
    }
}

// Insure the scrollbars are the last children:
void CScroll::fix_scrollbar_order()
{
    auto a = (Fl_Widget**) array();
    if (a[children() - 1] != &scrollbar) {
        int i, j;
        for (i = j = 0; j < children(); j++)
            if (a[j] != &hscrollbar && a[j] != &scrollbar)
                a[i++] = a[j];
        a[i++] = &hscrollbar;
        a[i++] = &scrollbar;
    }
}

void CScroll::draw_clip(void* v, int X, int Y, int W, int H)
{
    fl_push_clip(X, Y, W, H);
    auto s = (CScroll*) v;

    s->draw_box(s->box(), X, Y, W, H, s->color());

    Fl_Widget* const* a = s->array();
    int R = X;
    int B = Y; // track bottom & right edge of all children
    for (int i = s->children() - 2; i--;) {
        Fl_Widget& o = **a++;
        int NR, NB;
        s->draw_child(o);
        s->draw_outside_label(o);
        NR = o.x() + o.w();
        NB = o.y() + o.h();
        if ((o.align() & (FL_ALIGN_BOTTOM | FL_ALIGN_RIGHT)) &&
            !(o.align() & FL_ALIGN_INSIDE)) {
            int LW = 0, LH = 0;
            o.measure_label(LW, LH);
            if (o.align() & FL_ALIGN_BOTTOM)
                NB += LH;
            else
                NR += LW;
        }
        if (NR > R)
            R = NR;
        if (NB > B)
            B = NB;
    }
    // fill any area to right & bottom of widgets:
    if (R < X + W && B > Y) {
        s->draw_box(FL_FLAT_BOX, R, Y, X + W - R, B - Y, s->color());
    }
    if (B < Y + H) {
        s->draw_box(FL_FLAT_BOX, X, B, W, Y + H - B, s->color());
    }
    fl_pop_clip();
}

void CScroll::bbox(int& X, int& Y, int& W, int& H)
{
    X = x() + Fl::box_dx(box());
    Y = y() + Fl::box_dy(box());
    W = w() - Fl::box_dw(box());
    H = h() - Fl::box_dh(box());
    if (scrollbar.visible()) {
        W -= scrollbar.w();
        if (scrollbar.align() & FL_ALIGN_LEFT)
            X += scrollbar.w();
    }
    if (hscrollbar.visible()) {
        H -= hscrollbar.h();
        if (hscrollbar.align() & FL_ALIGN_TOP)
            Y += hscrollbar.h();
    }
}

void CScroll::draw()
{
    fix_scrollbar_order();
    int X, Y, W, H;
    bbox(X, Y, W, H);

    //uchar d = damage();

    draw_box(box(), x(), y(), w(), h(), color());
    draw_clip(this, X, Y, W, H);

    // accumulate bounding box of children:
    int l = X;
    int r = X;
    int t = Y;
    int b = Y;
    Fl_Widget* const* a = array();
    for (int i = children() - 2; i--;) {
        Fl_Object* o = *a++;
        if (o->x() < l)
            l = o->x();
        if (o->y() < t)
            t = o->y();
        if (o->x() + o->w() > r)
            r = o->x() + o->w();
        if (o->y() + o->h() > b)
            b = o->y() + o->h();
    }

    // turn the scrollbars on and off as necessary:
    // See if children would fit if we had no scrollbars...
    X = x() + Fl::box_dx(box());
    Y = y() + Fl::box_dy(box());
    W = w() - Fl::box_dw(box());
    H = h() - Fl::box_dh(box());
    int vneeded = 0;
    int hneeded = 0;
    if (type() & VERTICAL) {
        if ((type() & ALWAYS_ON) || t < Y || b > Y + H) {
            vneeded = 1;
            W -= scrollbar.w();
            if (scrollbar.align() & FL_ALIGN_LEFT)
                X += scrollbar.w();
        }
    }
    if (type() & HORIZONTAL) {
        if ((type() & ALWAYS_ON) || l < X || r > X + W) {
            hneeded = 1;
            H -= hscrollbar.h();
            if (scrollbar.align() & FL_ALIGN_TOP)
                Y += hscrollbar.h();
            // recheck vertical since we added a horizontal scrollbar
            if (!vneeded && (type() & VERTICAL)) {
                if ((type() & ALWAYS_ON) || t < Y || b > Y + H) {
                    vneeded = 1;
                    W -= scrollbar.w();
                    if (scrollbar.align() & FL_ALIGN_LEFT)
                        X += scrollbar.w();
                }
            }
        }
    }
    // Now that we know what's needed, make it so.
    if (vneeded && !scrollbar.visible()) {
        scrollbar.set_visible();
        //d = FL_DAMAGE_ALL;
    } else if (!vneeded && scrollbar.visible()) {
        scrollbar.clear_visible();
        draw_clip(this,
                  scrollbar.align() & FL_ALIGN_LEFT ? X : X + W - scrollbar.w(),
                  Y, scrollbar.w(), H);
        //d = FL_DAMAGE_ALL;
    }
    if (hneeded && !hscrollbar.visible()) {
        hscrollbar.set_visible();
        //d = FL_DAMAGE_ALL;
    } else if (!hneeded && hscrollbar.visible()) {
        hscrollbar.clear_visible();
        draw_clip(this,
                  X, scrollbar.align() & FL_ALIGN_TOP ? Y : Y + H - hscrollbar.h(),
                  W, hscrollbar.h());
        //d = FL_DAMAGE_ALL;
    }

    scrollbar.resize(scrollbar.align() & FL_ALIGN_LEFT ? X - scrollbar.w() : X + W, Y, scrollbar.w(), H);
    scrollbar.value(m_oldy = m_yposition = (Y - t), H, 0, b - t);

    hscrollbar.resize(X,
                      scrollbar.align() & FL_ALIGN_TOP ? Y - hscrollbar.h() : Y + H,
                      W, hscrollbar.h());
    hscrollbar.value(m_oldx = m_xposition = (X - l), W, 0, r - l);

    // draw the scrollbars:
    draw_child(scrollbar);
    draw_child(hscrollbar);
    if (scrollbar.visible() && hscrollbar.visible()) {
        // fill in the little box in the corner
        draw_box(FL_FLAT_BOX, scrollbar.x(), hscrollbar.y(), scrollbar.w(), hscrollbar.h(), color());
    }
}

void CScroll::resize(int xx, int yy, int ww, int hh)
{
    int dx = xposition();
    int dy = yposition();

    fix_scrollbar_order();
    // move all the children:
    Fl_Widget* const* a = array();
    for (int i = children() - 2; i--;) {
        Fl_Object* o = *a++;
        o->position(o->x() + xx - x(), o->y() + yy - y());
    }
    Fl_Widget::resize(xx, yy, ww, hh);

    int sbw = 0;
    int sbh = ww;
    scrollbar.preferredSize(sbw, sbh);
    ww -= sbw;
    hh -= sbw;
    scrollbar.size(sbw, scrollbar.h());
    hscrollbar.size(hscrollbar.w(), sbw);

    autoLayout(xx - dx, yy - dy, ww, hh, true);
    damage(FL_DAMAGE_ALL);
}

void CScroll::position(int X, int Y)
{
    int dx = m_xposition - X;
    int dy = m_yposition - Y;
    if (!dx && !dy)
        return;
    m_xposition = X;
    m_yposition = Y;
    Fl_Widget* const* a = array();
    for (int i = children(); i--;) {
        Fl_Widget* o = *a++;
        if (o == &hscrollbar || o == &scrollbar)
            continue;
        o->position(o->x() + dx, o->y() + dy);
    }
    damage(FL_DAMAGE_SCROLL);
}

void CScroll::hscrollbar_cb(Fl_Widget* o, void*)
{
    auto s = (CScroll*) (o->parent());
    s->position(((CScrollBar*) o)->value(), s->yposition());
}

void CScroll::scrollbar_cb(Fl_Widget* o, void*)
{
    auto s = (CScroll*) (o->parent());
    s->position(s->xposition(), ((CScrollBar*) o)->value());
}

int CScroll::handle(int event)
{
    switch (event) {
        case FL_KEYUP: {
            Fl_Widget* w = Fl::focus();
            if (w != nullptr) {
                Fl_Group* p = w->parent();
                while (p && p != this)
                    p = p->parent();
                if (p == this) {
                    int controlTop = w->y();
                    int controlBottom = controlTop + w->h();
                    int scrollTop = y();
                    int scrollBottom = scrollTop + h();
                    int xpos = xposition();
                    int ypos = yposition();

                    int dy = 0;
                    if (controlBottom > scrollBottom)
                        dy = controlBottom - scrollBottom;
                    if (controlTop - dy < scrollTop)
                        dy = controlTop - scrollTop;
                    ypos += dy;

                    position(xpos, ypos);
                }
            }
        }
            break;
        default:
            fix_scrollbar_order();
            break;
    }
    return Fl_Group::handle(event);
}
