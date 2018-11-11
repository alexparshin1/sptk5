/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CProgressBar.cpp - description                         ║
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
#include <sptk5/gui/CLayoutManager.h>
#include <sptk5/gui/CProgressBar.h>
#include <sptk5/gui/CThemes.h>
#include <cmath>

using namespace std;
using namespace sptk;

class CProgressBox : public Fl_Box
{
public:
    float m_value;
    float m_minValue;
    float m_maxValue;

    std::string m_units;
    bool m_showText;
public:
    CProgressBox(int x, int y, int w, int h)
            : Fl_Box(x, y, w, h)
    {
        m_value = 0;
        m_minValue = 0;
        m_maxValue = 100;
        m_showText = true;
        m_units = "%";
    }

    void draw() override;
};

void CProgressBox::draw()
{
    float delta = m_maxValue - m_minValue;
    if (!delta) {
        fl_draw_box(box(), x(), y(), w(), h(), parent()->color());
        return;
    }

    int fontHeight = 12;
    if (fontHeight >= h() - 4) fontHeight = h() - 4;

    fl_push_clip(x(), y(), w(), h() + 1);
    float percent = m_value * 100 / delta;
    if (percent < 0) percent = -percent;

    if (!CThemes::drawProgressBar(x(), y(), w(), percent)) {
        fl_draw_box(box(), x(), y(), w(), h(), parent()->color());
        auto width = (int) std::round((w() - Fl::box_dw(box())) * percent / 100.0);
        fl_draw_box(FL_THIN_UP_BOX, x() + Fl::box_dx(box()), y() + Fl::box_dy(box()), width, h() - Fl::box_dh(box()),
                    FL_BLUE);
    }

    if (m_showText) {
        std::string txt = int2string((int) m_value) + m_units;
        fl_color(FL_FOREGROUND_COLOR);
        fl_font(0, fontHeight);
        fl_draw(txt.c_str(), x(), y() + 1, w(), h(), Fl_Align(FL_ALIGN_CENTER));
    }
    fl_pop_clip();
}

void CProgressBar::ctor_init(const char* label)
{
    m_control = new CProgressBox(x(), y(), w(), h());
    add(m_control);
    m_control->box(FL_THIN_DOWN_BOX);
    m_control->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    m_control->callback(CControl::internalCallback);
    m_control->when(FL_WHEN_CHANGED);
    m_controlFlags = FGE_MULTILINEENTRY;
    labelWidth(0);
    m_label = label;
    data(0);
}

CProgressBar::CProgressBar(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        : CInput("", layoutSize, layoutAlignment, false)
{
    ctor_init(label);
}

#ifdef __COMPATIBILITY_MODE__
CProgressBar::CProgressBar(int x,int y,int w,int h,const char *label)
: CInput(x,y,w,h,"",false) {
   ctor_init(label);
}
#endif

CLayoutClient* CProgressBar::creator(xml::Node* node)
{
    auto widget = new CProgressBar("", 10, SP_ALIGN_TOP);
    widget->load(node, LXM_LAYOUTDATA);
    return widget;
}

bool CProgressBar::preferredSize(int& w, int& h)
{
    int minWidth, minHeight;
    CThemes::sizeProgressBar(minWidth, minHeight);
    if (minWidth) {
        // progress bar is themed
        if (w < minWidth)
            w = minWidth;
        h = minHeight;
    } else {
        auto pb = (CProgressBox*) m_control;
        if (w < 20) w = 20;
        if (pb->m_showText)
            h = m_textSize + 6;
        else if (h < 6) h = 6;
    }
    return false;
}

Variant CProgressBar::data() const
{
    auto pb = (CProgressBox*) m_control;
    return pb->m_value;
}

void CProgressBar::data(const Variant v)
{
    auto pb = (CProgressBox*) m_control;
    pb->m_value = v;
}

void CProgressBar::units(const char* un)
{
    auto pb = (CProgressBox*) m_control;
    pb->m_units = un;
}

std::string CProgressBar::units() const
{
    auto pb = (CProgressBox*) m_control;
    return pb->m_units;
}

void CProgressBar::showText(bool flag)
{
    auto pb = (CProgressBox*) m_control;
    pb->m_showText = flag;
}

void CProgressBar::minimum(float value)
{
    auto pb = (CProgressBox*) m_control;
    pb->m_minValue = value;
}

float CProgressBar::minimum() const
{
    auto pb = (CProgressBox*) m_control;
    return pb->m_minValue;
}

void CProgressBar::maximum(float value)
{
    auto pb = (CProgressBox*) m_control;
    pb->m_maxValue = value;
}

float CProgressBar::maximum() const
{
    auto pb = (CProgressBox*) m_control;
    return pb->m_maxValue;
}
