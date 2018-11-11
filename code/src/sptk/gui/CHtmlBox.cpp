/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CHtmlBox.cpp - description                             ║
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

#include <FL/Fl_Help_View.H>

#include <sptk5/gui/CHtmlBox.h>

using namespace sptk;

//===========================================================================
void CHtmlBox::ctor_init(const char* label)
{
    m_controlFlags = FGE_MULTILINEENTRY;
    m_control = new Fl_Help_View(x(), y(), w(), h(), "");
    add(m_control);
    m_control->box(FL_FLAT_BOX);
    m_control->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
    m_control->callback(CControl::internalCallback);
    m_control->when(FL_WHEN_CHANGED);
    labelWidth(0);
    if (label)
        data(label);
}

CHtmlBox::CHtmlBox(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        : CInput("", layoutSize, layoutAlignment, false)
{
    ctor_init(label);
}

#ifdef __COMPATIBILITY_MODE__
CHtmlBox::CHtmlBox(int x,int y,int w,int h,const char *label)
        : CInput(x,y,w,h,"",false) {
    ctor_init(label);
}
#endif

CLayoutClient* CHtmlBox::creator(xml::Node* node)
{
    auto widget = new CHtmlBox("", 10, SP_ALIGN_TOP);
    widget->load(node, LXM_LAYOUTDATA);
    return widget;
}

Variant CHtmlBox::data() const
{
    return ((Fl_Help_View*) m_control)->value();
}

void CHtmlBox::data(const Variant v)
{
    ((Fl_Help_View*) m_control)->value(v.getString());
}

Fl_Font CHtmlBox::textFont() const
{
    return Fl_Font(((Fl_Help_View*) m_control)->textfont());
}

void CHtmlBox::textFont(Fl_Font f)
{
    ((Fl_Help_View*) m_control)->textfont(f);
}

uchar CHtmlBox::textSize() const
{
    return (uchar) ((Fl_Help_View*) m_control)->textsize();
}

void CHtmlBox::textSize(uchar s)
{
    ((Fl_Help_View*) m_control)->textsize(s);
}

int CHtmlBox::totalHeight() const
{
    return ((Fl_Help_View*) m_control)->size();
}

bool CHtmlBox::preferredSize(int& w, int& h)
{
    auto hv = (Fl_Help_View*) m_control;
    hv->resize(hv->x(), hv->y(), w, h);
    int totalHeight = hv->size();

    int hh = totalHeight + 10;
    if (hh < (int) labelHeight())
        hh = labelHeight();
    if (h < hh)
        h = hh;
    if (w < (int) m_labelWidth + 16)
        w = m_labelWidth + 16;

    return false;
}
