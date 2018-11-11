/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CTreeView.cpp - description                            ║
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

#include <sptk5/gui/CDataControl.h>
#include <sptk5/gui/CTreeControl.h>
#include <sptk5/gui/CTreeView.h>

using namespace std;
using namespace sptk;

void CTreeView::internal_callback(Fl_Widget* widget, void*)
{
    auto tree = (CTreeView*) widget->parent();
    tree->do_callback();
}

void CTreeView::ctor_init()
{
    m_controlFlags = FGE_MULTILINEENTRY;
    m_treeControl = new CTreeControl("", 10, SP_ALIGN_NONE);
    m_control = m_treeControl;
    m_control->callback(CControl::internalCallback);
    labelWidth(0);
    end();
}

CTreeView::CTreeView(const char* label, int layoutSize, CLayoutAlign layoutAlignment)
    : CControl(label, layoutSize, layoutAlignment)
{
    ctor_init();
}

#ifdef __COMPATIBILITY_MODE__
CTreeView::CTreeView(int x,int y,int w,int h,const char *label)
        : CControl(x,y,w,h,label) {
    ctor_init();
}
#endif

CLayoutClient* CTreeView::creator(xml::Node* node)
{
    auto widget = new CTreeView("", 10, SP_ALIGN_TOP);
    widget->load(node);
    return widget;
}

String CTreeView::selectedPath() const
{
    std::string result;
    CTreeItem* node = m_treeControl->selected();
    if (node == nullptr)
        return "";
    do {
        result = "/" + std::string(node->label()) + result;
        node = node->parentItem();
    } while (node != nullptr && node->parentItem() != nullptr);
    return result;
}

void CTreeView::resize(int x, int y, int w, int h)
{
    CControl::resize(x, y, w, h);

    w -= m_labelWidth;
    x += m_labelWidth;

    if (m_menuButton != nullptr)
        m_menuButton->resize(x, y, w, this->h());

    m_treeControl->resize(x, y, w, h);
}

void CTreeView::load(Query* loadQuery)
{
    if (m_fieldName.empty()) return;
    Field& fld = (*loadQuery)[m_fieldName.c_str()];
    data(*(Variant*)&fld);
}

void CTreeView::save(Query* updateQuery)
{
    if (m_fieldName.empty()) return;
    QueryParameter& param = updateQuery->param(m_fieldName.c_str());
    param = data();
}

void CTreeView::load(const xml::Node* node)
{
    m_treeControl->load(node);
}

void CTreeView::save(xml::Node* node) const
{
    m_treeControl->save(node);
}

Variant CTreeView::data() const
{
    CTreeItem* node = m_treeControl->selected();
    if (node != nullptr) return (int32_t) (long) node->user_data();
    return int32_t(0);
}

void CTreeView::data(const Variant v)
{
    CTreeItem* node = m_treeControl->findData((void*) (long) v.asInteger());
    if (node != nullptr)
        m_treeControl->makeVisible(node);
}
