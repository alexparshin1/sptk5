/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CCheckButtons.cpp - description                        ║
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
#include <FL/Fl_Round_Button.H>
#include <FL/fl_draw.H>

#include <sptk5/gui/CDataControl.h>
#include <sptk5/gui/CCheckButtons.h>
#include <sptk5/gui/CThemes.h>
#include <sptk5/gui/CScroll.h>

using namespace std;
using namespace sptk;

void CCheckButton::draw()
{
    int bh = 0;
    int bw = 0;
    CThemes::sizeButton(THM_BUTTON_CHECK, bw, bh);
    bool checked = value() != 0;
    bool highlited = (Fl::belowmouse() == this) && active_r();
    int dy = (h() - bh) / 2;
    if (CThemes::drawCheckButton(x(), y() + dy, checked, highlited)) {
        if (!m_label.empty()) {
            fl_color(labelcolor());
            fl_font(labelfont(), labelsize());
            fl_draw(m_label.c_str(), x() + bw + 4, y() + 2, w() - bw - 6, h() - 4,
                    Fl_Align(FL_ALIGN_LEFT | FL_ALIGN_WRAP));
            if (Fl::focus() == this)
                draw_focus(FL_FLAT_BOX, x() + bw + 2, y() + 1, w() - bw - 2, h() - 1);
        }
    } else
        Fl_Check_Button::draw();
}

bool CCheckButton::preferredSize(int& w, int& h)
{
    int bh = 12;
    int bw = 12;
    CThemes::sizeButton(THM_BUTTON_CHECK, bw, bh);
    fl_font(labelfont(), labelsize());
    w -= bw + 6;
    fl_measure(m_label.c_str(), w, h);
    w += bw + 6;
    if (h < bh)
        h = bh;
    if (h < labelsize())
        h = labelsize();
    return false;
}

int CCheckButton::handle(int event)
{
    CControl* control;
    switch (event) {
        case FL_FOCUS:
            control = dynamic_cast<CControl*> (parent());
            if (!control && parent())
                control = dynamic_cast<CControl*> (parent()->parent());
            if (control)
                control->notifyFocus();
            break;
        case FL_ENTER:
        case FL_LEAVE:
            redraw();
            break;
        default:
            break;
    }
    return Fl_Button::handle(event);
}

void CCheckButtons::checkButtonsCallback(Fl_Widget* w, void*)
{
    auto b = (Fl_Button*) w;
    Fl_Group* g = b->parent();
    auto c = (CCheckButtons*) g->parent();
    c->controlDataChanged();
    g->redraw();
}

CCheckButtons::CCheckButtons(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        : CRadioButtons(label, layoutSize, layoutAlignment)
{
}

#ifdef __COMPATIBILITY_MODE__

CCheckButtons::CCheckButtons(int x, int y, int w, int h, const char *l)
: CRadioButtons(x, y, w, h, l) {
}
#endif

Fl_Button* CCheckButtons::createButton(const char* label, int sz, CLayoutAlign layoutAlignment)
{
    Fl_Button* btn = new CCheckButton(label, sz, layoutAlignment);
    btn->callback(CCheckButtons::checkButtonsCallback);
    return btn;
}

Variant CCheckButtons::data() const
{
    string result;
    auto group = (CScroll*) m_control;
    auto cnt = (unsigned) group->children();
    for (unsigned i = 0; i < cnt; i++) {
        auto b = dynamic_cast<Fl_Button*> (group->child(i));
        if (!b)
            continue;
        if (b->value()) {
            if (b == m_otherButton) {
                result += m_otherInput->value();
            } else {
                result += b->label();
            }
            result += "|";
        }
    }
    size_t len = result.length();
    if (len)
        result[len - 1] = 0;

    return result;
}

void CCheckButtons::data(const Variant s)
{
    deselectAllButtons();
    if (m_otherButton)
        m_otherInput->value("");
    string st = s;
    Strings sl(st, "|");
    size_t cnt = sl.size();
    auto g = (CScroll*) m_control;
    for (unsigned i = 0; i < cnt; i++) {
        int ndx = buttonIndex(sl[i].c_str());
        if (ndx > -1) {
            auto b = (Fl_Button*) g->child(ndx);
            b->value(1);
        } else {
            if (m_otherButton) {
                m_otherButton->value(1);
                m_otherInput->value(sl[i].c_str());
            }
        }
    }
}

void CCheckButtons::getSelections(IntList& selection) const
{
    selection.clear();
    auto group = (CScroll*) m_control;
    auto cnt = (unsigned) group->children();
    for (unsigned i = 0; i < cnt; i++) {
        auto b = dynamic_cast<Fl_Button*> (group->child(i));
        if (!b)
            continue;
        if (b->value()) {
            auto id = (uint32_t) (long) b->user_data();
            selection.push_back(id);
        }
    }
}

void CCheckButtons::setSelections(const IntList& selection)
{
    deselectAllButtons();
    auto group = (CScroll*) m_control;
    auto cnt = (unsigned) group->children();
    for (unsigned i = 0; i < cnt; i++) {
        auto b = dynamic_cast<Fl_Button*> (group->child(i));
        if (!b)
            continue;
        auto id = (uint32_t) (long) b->user_data();
        auto itor = std::find(selection.begin(), selection.end(), id);
        if (itor != selection.end())
            b->value(1);
    }
}

CLayoutClient* CCheckButtons::creator(xml::Node* node)
{
    auto widget = new CCheckButtons("", 20, SP_ALIGN_NONE);
    widget->load(node, LXM_LAYOUTDATA);
    return widget;
}
