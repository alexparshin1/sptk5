/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CRadioButtons.cpp - description                        ║
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

#include <sptk5/gui/CScroll.h>
#include <sptk5/gui/CDataControl.h>
#include <sptk5/gui/CRadioButtons.h>
#include <sptk5/gui/CThemes.h>

using namespace std;
using namespace sptk;

namespace sptk {

    class CRadioButton : public Fl_Round_Button, public CLayoutClient
    {
    protected:
        void draw() override;

    public:
        CRadioButton(const char* label, int layoutSize, CLayoutAlign layoutAlignment);

        int handle(int) override;

        bool preferredSize(int& w, int& h) override;
    };

}

CRadioButton::CRadioButton(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        : Fl_Round_Button(0, 0, layoutSize, layoutSize, label), CLayoutClient(this, layoutSize, layoutAlignment)
{}

void CRadioButton::draw()
{
    int bh = 0;
    int bw = 0;
    CThemes::sizeButton(THM_BUTTON_RADIO, bw, bh);
    bool checked = value() != 0;
    bool highlited = (Fl::belowmouse() == this) && active_r();
    int dy = (h() - bh) / 2;
    if (CThemes::drawRadioButton(x(), y() + dy, checked, highlited)) {
        if (!m_label.empty()) {
            fl_color(labelcolor());
            fl_font(labelfont(), labelsize());
            fl_draw(m_label.c_str(), x() + bw + 4, y() + 2, w() - bw - 6, h() - 4,
                    Fl_Align(FL_ALIGN_LEFT | FL_ALIGN_WRAP));
            if (Fl::focus() == this)
                draw_focus(FL_FLAT_BOX, x() + bw + 2, y() + 1, w() - bw - 2, h() - 1);
        }
    } else
        Fl_Round_Button::draw();
}

bool CRadioButton::preferredSize(int& w, int& h)
{
    int bh = 12;
    int bw = 12;
    CThemes::sizeButton(THM_BUTTON_RADIO, bw, bh);

    fl_font(labelfont(), labelsize());
    w -= bw + 6;
    fl_measure(m_label.c_str(), w, h);
    w += bw + 6;
    if (h < labelsize())
        h = labelsize();
    if (h < bh)
        h = bh;
    return false;
}

int CRadioButton::handle(int event)
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

void CRadioButtons::radioButtonsCallback(Fl_Widget* w, void*)
{
    auto b = (Fl_Button*) w;
    Fl_Group* g = b->parent();
    auto r = (CRadioButtons*) g->parent();
    r->deselectAllButtons();
    b->value(1);
    g->redraw();
    r->controlDataChanged();
}

CRadioButtons::CRadioButtons(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        : CButtonGroup(label, layoutSize, layoutAlignment)
{}

#ifdef __COMPATIBILITY_MODE__
CRadioButtons::CRadioButtons(int x,int y,int w,int h,const char *l)
        : CButtonGroup (x,y,w,h,l) {}
#endif

CLayoutClient* CRadioButtons::creator(xml::Node* node)
{
    auto widget = new CRadioButtons("", 10, SP_ALIGN_TOP);
    widget->load(node, LXM_LAYOUTDATA);
    return widget;
}

Fl_Button* CRadioButtons::createButton(const char* label, int sz, CLayoutAlign layoutAlignment)
{
    Fl_Button* btn = new CRadioButton(label, sz, layoutAlignment);
    btn->callback(CRadioButtons::radioButtonsCallback);
    btn->when(FL_WHEN_CHANGED);
    return btn;
}

int32_t CRadioButtons::intValue() const
{
    auto group = (CScroll*) m_control;
    auto cnt = (unsigned) group->children();
    for (uint32_t i = 0; i < cnt; i++) {
        auto b = dynamic_cast<Fl_Button*>(group->child(i));
        if (!b)
            continue;
        if (b->value())
            return (int32_t) (long) b->user_data();
    }
    return 0;
}

void CRadioButtons::intValue(int32_t v)
{
    deselectAllButtons();
    auto group = (CScroll*) m_control;
    auto cnt = (unsigned) group->children();
    for (unsigned i = 0; i < cnt; i++) {
        auto b = dynamic_cast<Fl_Button*>(group->child(i));
        if (!b)
            continue;
        if (long(b->user_data()) == v) {
            b->value(1);
            break;
        }
    }
}
