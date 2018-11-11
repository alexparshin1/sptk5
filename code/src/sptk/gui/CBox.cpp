/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CBox.cpp - description                                 ║
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

#include <FL/Fl.H>
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <sptk5/Strings.h>
#include <sptk5/gui/CBox.h>

using namespace sptk;

//===========================================================================
void CBox::ctor_init(const char* label)
{
    m_controlFlags = FGE_MULTILINEENTRY;
    begin();
    m_control = new Fl_Box(x(), y(), w(), h());
    m_control->clear_output();
    end();
    align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    m_control->callback(CControl::internalCallback);
    m_control->when(FL_WHEN_CHANGED);
    labelWidth(0);
    if (label != nullptr)
        data(label);
}

CBox::CBox(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        : CInput("", layoutSize, layoutAlignment, false)
{
    ctor_init(label);
}

#ifdef __COMPATIBILITY_MODE__
CBox::CBox(int x,int y,int w,int h,const char *label)
        : CInput(x,y,w,h,"",false) {
    ctor_init(label);
}
#endif

CLayoutClient* CBox::creator(xml::Node* node)
{
    auto widget = new CBox("", 10, SP_ALIGN_TOP);
    widget->load(node, LXM_LAYOUTDATA);
    widget->dragable(node->getAttribute("drag", "N"));
    return widget;
}

void CBox::draw()
{
    m_control->callback(callback());
    Fl_Widget::color(m_control->color());
    draw_box();
    m_control->align(Fl_Widget::align());
    m_control->labelcolor(labelColor());
    m_control->label(m_label.c_str());
    m_control->labelfont(labelfont());
    m_control->labelsize(labelsize());
    m_control->labeltype(labeltype());
    m_control->draw();
}

Variant CBox::data() const
{
    return m_label.c_str();
}

void CBox::data(const Variant v)
{
    String text = v.asString();
    m_label.resize(text.length());
    if (!m_label.empty()) {
        unsigned i = 0;
        for (auto& c: text) {
            if (c == '\r')
                continue;
            m_label[i] = c;
            i++;
        }
        m_label[i] = 0;

        m_label = m_label.replace("\x0B", "  ");
    }
    m_control->label(m_label.c_str());
}

Fl_Font CBox::textFont() const
{
    return m_control->labelfont();
}

void CBox::textFont(Fl_Font f)
{
    m_control->labelfont(f);
}

uchar CBox::textSize() const
{
    return (uchar) m_control->labelsize();
}

void CBox::textSize(uchar s)
{
    m_control->labelsize(s);
}

unsigned CBox::textAlign() const
{
    return m_control->align();
}

void CBox::textAlign(unsigned align)
{
    m_control->align(align);
}

int CBox::totalHeight(int ww) const
{
    fl_font(m_textFont, m_textSize);
    if (ww < 1)
        ww = w();
    int cw = ww, ch = 0;
    fl_measure(m_label.c_str(), cw, ch);
    return ch;
}

void CBox::resize(int x, int y, int w, int h)
{
    m_labelWidth = 0;
    Fl_Group::resize(x, y, w, h);
    m_control->resize(x, y, w, h);
}

bool CBox::preferredSize(int& w, int& h)
{
    int cw = 0, ch = 0;
    int dh = Fl::box_dh(box()) * 2;
    int dw = Fl::box_dw(box()) * 2;

    switch (m_layoutAlign) {
        case SP_ALIGN_TOP:
        case SP_ALIGN_BOTTOM:
            if ((align() & FL_ALIGN_WRAP) == 0)
                cw = 0;
            else
                cw = w - dw;
            break;
        case SP_ALIGN_LEFT:
        case SP_ALIGN_RIGHT:
            ch = h - dh;
            break;
        default:
            break;
    }

    fl_font(m_textFont, m_textSize);
    fl_measure(m_label.c_str(), cw, ch);
    int hh = ch + dh;
    int ww = cw + dw;
    if (h < hh)
        h = hh;
    if (w < ww)
        w = ww;
    return false;
}

int CBox::handle(int event)
{
    switch (event) {
        case FL_PUSH:
            m_xPushed = Fl::event_x();
            m_yPushed = Fl::event_y();
            fireEvent(CE_MOUSE_CLICK, 0);
            return 1;
        case FL_DRAG:
            if (m_dragable) {
                int dx = Fl::event_x_root();
                int dy = Fl::event_y_root();
                Fl_Window* w = window();
                w->position(dx - m_xPushed, dy - m_yPushed);
                return 1;
            }
            return 0;
        case FL_RELEASE:
            return 1;
        default:
            break;
    }
    return Fl_Widget::handle(event);
}
