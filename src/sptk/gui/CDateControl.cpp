/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CDateControl.cpp - description                         ║
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

#include <sptk5/gui/CDateControl.h>
#include <sptk5/gui/CInput.h>
#include <sptk5/gui/CButton.h>

using namespace std;
using namespace sptk;

void CDateControl::date_cb(Fl_Widget* w, void*)
{
    auto* dateControl = (CDateControl*) w->parent();
    dateControl->do_callback(dateControl, dateControl->user_data());
}

CDateControl::CDateControl(int x, int y, int w, int h, const char* label)
        : Fl_Group(x, y, w, h, label)
{
    box(FL_DOWN_BOX);
    m_input = new CInput_;
    m_input->mask(DateTime::dateFormat);
    m_input->box(FL_FLAT_BOX);
    m_input->callback(date_cb);
    m_input->when(FL_WHEN_CHANGED);
    m_button = new CSmallButton(SP_CALENDAR_BUTTON);
    m_button->visible_focus(false);
    end();
}

void CDateControl::resize(int x, int y, int w, int h)
{
    int editorWidth;
    int buttonWidth;
    preferredSize(w, h, editorWidth, buttonWidth);
    Fl_Group::resize(x, y, w, h);
    int dx = Fl::box_dx(box());
    int dy = Fl::box_dy(box());
    int dh = Fl::box_dh(box());
    m_input->resize(x + dx, y + dy, editorWidth, h - dh);
    m_button->resize(x + editorWidth + dx, y + dy, buttonWidth, h - dh);
}

void CDateControl::preferredHeight(int& hh)
{
    auto parentControl = dynamic_cast<CControl*>(parent());
    int bw = 18;
    int bh = 18;
    int dh = Fl::box_dh(box());
    CThemes::sizeButton(THM_BUTTON_COMBO, bw, bh);
    if (parentControl) {
        hh = parentControl->textSize() + 8;
        if (hh < bh) hh = bh;
    }
    hh += dh;
}

void CDateControl::preferredSize(int& ww, int& hh, int& editorWidth, int& buttonWidth)
{
    auto parentControl = dynamic_cast<CControl*>(parent());
    editorWidth = 70;
    int bw = 18;
    int bh = 18;
    CThemes::sizeButton(THM_BUTTON_COMBO, bw, bh);
    int dw = Fl::box_dw(box());
    if (parentControl) {
        fl_font(parentControl->textFont(), parentControl->textSize());
        editorWidth = (int) fl_width("00/00/00000");
    }
    buttonWidth = bw;
    ww = editorWidth + bw + dw;
    preferredHeight(hh);
}
