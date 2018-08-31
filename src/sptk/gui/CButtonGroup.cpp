/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CButtonGroup.cpp - description                         ║
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

#include <sptk5/cgui>

using namespace std;
using namespace sptk;

void CButtonGroup::ctor_init()
{
    m_controlFlags = FGE_MULTILINEENTRY;
    m_maxHeight = 0;
    m_control = new CGroup;
    m_control->align(FL_ALIGN_LEFT);
    end();
    clearButtons();
}

CButtonGroup::CButtonGroup(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
        : CControl(label, layoutSize, layoutAlignment)
{
    ctor_init();
}

#ifdef __COMPATIBILITY_MODE__
CButtonGroup::CButtonGroup(int x,int y,int w,int h,const char *l)
: CControl (x,y,w,h,l) 
{
   ctor_init();
}
#endif

void CButtonGroup::controlDataChanged()
{
    string newValue = data().getString();
    if (m_lastValue != newValue)
        fireEvent(CE_DATA_CHANGED, 0);
}

void CButtonGroup::clearButtons()
{
    auto group = (CGroup*) m_control;
    group->clear();
    m_buttonLabels.clear();
    m_otherInput = nullptr;
    m_otherButton = nullptr;
}

void CButtonGroup::buttons(const Strings& sl)
{
    clearButtons();
    m_buttonLabels = sl;
    size_t buttonsCount = m_buttonLabels.size();
    auto group = (CGroup*) m_control;
    group->begin();
    const String* otherLabel = nullptr;
    for (unsigned i = 0; i < buttonsCount; i++) {
        String& si = m_buttonLabels[i];
        if (si == "*") {
            if (otherLabel) continue;  // Only one free entry is allowed
            otherLabel = &m_buttonLabels[i];
        } else {
            Fl_Button* btn = createButton(si.c_str());
            btn->argument(si.ident());
            btn->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT | FL_ALIGN_WRAP);
        }
    }
    if (otherLabel) {
        Fl_Button* btn = createButton("", 16, SP_ALIGN_LEFT);
        btn->argument(otherLabel->ident());
        m_otherButton = btn;
        m_otherInput = new CInput_("");
        m_otherInput->color(FL_LIGHT3);
    }
    end();
    resize(x(), y(), w(), h());
}

Variant CButtonGroup::data() const
{
    auto group = (CGroup*) m_control;
    auto cnt = unsigned(group->children());
    for (unsigned i = 0; i < cnt; i++) {
        Fl_Widget* w = group->child(i);
        auto b = dynamic_cast<Fl_Button*>(w);
        if (!b) continue;
        if (b->value()) {
            if (b == m_otherButton) {
                return m_otherInput->value();
            } else {
                return b->label();
            }
        }
    }
    return Variant("");
}

int CButtonGroup::buttonIndex(const char* buttonLabel)
{
    auto group = (CGroup*) m_control;
    auto cnt = (unsigned) group->children();
    for (unsigned i = 0; i < cnt; i++) {
        auto b = dynamic_cast<Fl_Button*>(group->child(i));
        if (!b) continue;
        if (strcmp(b->label(), buttonLabel) == 0)
            return i;
    }
    return -1;
}

void CButtonGroup::deselectAllButtons()
{
    auto group = (CGroup*) m_control;
    auto cnt = unsigned(group->children());
    for (unsigned i = 0; i < cnt; i++) {
        auto b = dynamic_cast<Fl_Button*>(group->child(i));
        if (!b) continue;
        b->value(0);
    }
}

void CButtonGroup::data(const Variant d)
{
    deselectAllButtons();
    int ndx = buttonIndex(d.getString());
    if (ndx > -1) {
        auto g = (CGroup*) m_control;
        auto b = (Fl_Button*) g->child(ndx);
        b->value(1);
    } else {
        if (m_otherButton) {
            m_otherButton->value(1);
            m_otherInput->value(d.getString());
        }
    }
    fireEvent(CE_DATA_CHANGED, 0);
}

bool CButtonGroup::preferredSize(int& w, int& h)
{
    auto g = (CGroup*) m_control;
    w -= labelWidth();
    bool rc = g->preferredSize(w, h);
    w += labelWidth();
    int hh = labelHeight();
    if (h < hh) h = hh;
    return rc;
}

void CButtonGroup::load(Query* loadQuery)
{
    if (!m_fieldName.length()) return;
    Field& fld = (*loadQuery)[m_fieldName.c_str()];
    data(fld.asString());
}

void CButtonGroup::save(Query* updateQuery)
{
    if (!m_fieldName.length()) return;
    QueryParameter& param = updateQuery->param(m_fieldName.c_str());
    param = data();
}

void CButtonGroup::load(const xml::Node* node, CLayoutXMLmode xmlMode)
{
    CControl::load(node, xmlMode);
    Strings btns(node->getAttribute("buttons"), "|");
    if (!btns.empty())
        buttons(btns);
}

void CButtonGroup::save(xml::Node* node, CLayoutXMLmode xmlMode) const
{
    CControl::save(node, xmlMode);
    node->setAttribute("buttons", buttons().asString("|"));
}
