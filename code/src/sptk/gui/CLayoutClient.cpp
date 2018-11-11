/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CLayoutClient.cpp - description                        ║
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

#include <sptk5/gui/CLayoutClient.h>
#include <sptk5/gui/CLayoutManager.h>
#include <sptk5/gui/CControl.h>

using namespace std;
using namespace sptk;

CLayoutClient::CLayoutClient(Fl_Widget* widget, int layoutSize, CLayoutAlign ca)
{
    m_lastPreferredW = m_lastPreferredH = 0;
    m_layoutSize = layoutSize;
    m_layoutAlign = ca;
    m_widget = widget;
    if (widget->label()) {
        m_label = widget->label();
        widget->label(m_label.c_str());
    }
}

void CLayoutClient::load(const xml::Node* node, CLayoutXMLmode xmlMode)
{
    if (xmlMode & (int) LXM_LAYOUT) {
        CLayoutAlign layoutAlign;
        String alignName(lowerCase(node->getAttribute("layout_align")));
        switch (alignName[0]) {
            case 'b':
                layoutAlign = SP_ALIGN_BOTTOM;
                break;
            case 'l':
                layoutAlign = SP_ALIGN_LEFT;
                break;
            case 'r':
                layoutAlign = SP_ALIGN_RIGHT;
                break;
            case 'c':
                layoutAlign = SP_ALIGN_CLIENT;
                break;
            case 't':
                layoutAlign = SP_ALIGN_TOP;
                break;
            default:
                layoutAlign = SP_ALIGN_NONE;
                {
                    int x = node->getAttribute("x", "-1");
                    int y = node->getAttribute("y", "-1");
                    int w = node->getAttribute("w", "-1");
                    int h = node->getAttribute("h", "-1");
                    if (x > -1 && y > -1)
                        m_widget->position(x, y);
                    if (w > -1 && h > -1)
                        m_widget->Fl_Widget::size(w, h);
                }
                break;
        }
        m_layoutAlign = layoutAlign;
        name(node->getAttribute("name"));
        label(node->getAttribute("label"));

        if (layoutAlign != SP_ALIGN_NONE) {
            int layoutSize = node->getAttribute("layout_size");
            if (layoutSize)
                m_layoutSize = layoutSize;
        }

        string boxTypeName = node->getAttribute("box");
        if (boxTypeName.empty())
            boxTypeName = (string) node->getAttribute("frame");
        if (!boxTypeName.empty()) {
            auto btor = CLayoutManager::boxTypeNames().find(boxTypeName);
            if (btor != CLayoutManager::boxTypeNames().end())
                m_widget->box(btor->second);
        }

        if (!(bool) node->getAttribute("visible", "Y"))
            m_widget->hide();
        else
            m_widget->show();
        if (!(bool) node->getAttribute("enable", "Y"))
            m_widget->deactivate();
        else
            m_widget->activate();
    }
    if (xmlMode & (int) LXM_DATA) {
        auto control = dynamic_cast<CControl*>(m_widget);
        if (control)
            control->load(node, LXM_DATA);
    }
}

void CLayoutClient::save(xml::Node* node, CLayoutXMLmode xmlMode) const
{
    if (!node->isElement())
        throw Exception("Node must be an element");
    String className = "widget";
    auto layoutClient = dynamic_cast<CLayoutClient*>(m_widget);
    if (layoutClient)
        className = layoutClient->className();
    node->name(className);

    if (xmlMode & (int) LXM_LAYOUT) {
        if (!m_name.empty())
            node->setAttribute("name", name());
        if (!m_label.empty())
            node->setAttribute("label", label());
        if (!m_widget->visible())
            node->setAttribute("visible", m_widget->visible());
        if (!m_widget->active())
            node->setAttribute("enable", m_widget->active());

        String layoutAlignStr;
        switch (m_layoutAlign) {
            case SP_ALIGN_TOP:
                layoutAlignStr = "top";
                break;
            case SP_ALIGN_BOTTOM:
                layoutAlignStr = "bottom";
                break;
            case SP_ALIGN_LEFT:
                layoutAlignStr = "left";
                break;
            case SP_ALIGN_RIGHT:
                layoutAlignStr = "right";
                break;
            case SP_ALIGN_CLIENT:
                layoutAlignStr = "client";
                break;
            case SP_ALIGN_NONE:
                break;
        }

        if (layoutAlignStr.empty()) {
            node->setAttribute("x", m_widget->x());
            node->setAttribute("y", m_widget->y());
            node->setAttribute("w", m_widget->w());
            node->setAttribute("h", m_widget->h());
        } else {
            node->setAttribute("layout_align", layoutAlignStr);
            node->setAttribute("layout_size", layoutSize());
        }
    } else {
        if (!m_name.empty())
            node->setAttribute("name", name());
        else if (!m_label.empty())
            node->setAttribute("label", label());
    }
    if (xmlMode & (int) LXM_DATA) {
        auto control = dynamic_cast<CControl*>(m_widget);
        if (control != nullptr)
            control->save(node, LXM_DATA);
    }
}
