/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CMemoInput.cpp - description                           ║
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

#include <sptk5/gui/CMemoInput.h>
#include <sptk5/gui/CEditor.h>

using namespace sptk;

void CMemoInput::ctor_init()
{
    m_controlFlags = FGE_MULTILINEENTRY;
    begin();
    auto e = new CEditor(50, SP_ALIGN_NONE);
    e->wrap_mode(true, 0);
    m_control = e;
    m_control->callback(CControl::internalCallback);
    m_control->when(FL_WHEN_CHANGED);
    end();
}

CMemoInput::CMemoInput(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
    : CInput(label, layoutSize, layoutAlignment, false)
{
    ctor_init();
}

#ifdef __COMPATIBILITY_MODE__
CMemoInput::CMemoInput(int x,int y,int w,int h,const char * label)
        : CInput(x,y,w,h,label,false) {
    ctor_init();
}
#endif

CLayoutClient* CMemoInput::creator(xml::Node* node)
{
    auto widget = new CMemoInput("", 10, SP_ALIGN_TOP);
    widget->load(node, LXM_LAYOUTDATA);
    return widget;
}

Variant CMemoInput::data() const
{
    return ((CEditor*) m_control)->textBuffer()->text();
}

void CMemoInput::data(const Variant s)
{
    String strValue = s.asString().replace("\\r", "");
    ((CEditor*) m_control)->textBuffer()->text(strValue.c_str());
}

Fl_Font CMemoInput::textFont() const
{
    return ((CEditor*) m_control)->textfont();
}

void CMemoInput::textFont(Fl_Font f)
{
    ((CEditor*) m_control)->textfont(f);
}

uchar CMemoInput::textSize() const
{
    return (uchar) ((CEditor*) m_control)->textsize();
}

void CMemoInput::textSize(uchar s)
{
    ((CEditor*) m_control)->textsize(s);
}

void CMemoInput::save(Query* updateQuery)
{
    if (m_fieldName.empty())
        return;
    QueryParameter& param = updateQuery->param(m_fieldName.c_str());
    param.setText(data().getString());
}

bool CMemoInput::preferredSize(int& w, int& h)
{
    if (h < 30)
        h = 30;
    if (w < 30)
        w = 30;
    return false;
}
