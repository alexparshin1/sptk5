/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2022 Alexey Parshin. All rights reserved.       ║
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
    : m_layoutSize(layoutSize), m_widget(widget), m_layoutAlign(ca)
{
    if (widget->label())
    {
        m_label = widget->label();
        widget->label(m_label.c_str());
    }
}

void CLayoutClient::load(const xdoc::SNode& node, CLayoutXMLmode xmlMode)
{
    if ((int) xmlMode & (int) CLayoutXMLmode::LAYOUT)
    {
        CLayoutAlign layoutAlign;
        String alignName(lowerCase((String) node->attributes().get("layout_align")));
        switch (alignName[0])
        {
            case 'b':
                layoutAlign = CLayoutAlign::BOTTOM;
                break;
            case 'l':
                layoutAlign = CLayoutAlign::LEFT;
                break;
            case 'r':
                layoutAlign = CLayoutAlign::RIGHT;
                break;
            case 'c':
                layoutAlign = CLayoutAlign::CLIENT;
                break;
            case 't':
                layoutAlign = CLayoutAlign::TOP;
                break;
            default:
                layoutAlign = CLayoutAlign::NONE;
                {
                    int x = node->attributes().get("x", "-1").toInt();
                    int y = node->attributes().get("y", "-1").toInt();
                    int w = node->attributes().get("w", "-1").toInt();
                    int h = node->attributes().get("h", "-1").toInt();
                    if (x > -1 && y > -1)
                    {
                        m_widget->position(x, y);
                    }
                    if (w > -1 && h > -1)
                    {
                        m_widget->Fl_Widget::size(w, h);
                    }
                }
                break;
        }
        m_layoutAlign = layoutAlign;
        name((String) node->attributes().get("name"));
        label((String) node->attributes().get("label"));

        if (layoutAlign != CLayoutAlign::NONE)
        {
            int layoutSize = node->attributes().get("layout_size").toInt();
            if (layoutSize)
            {
                m_layoutSize = layoutSize;
            }
        }

        String boxTypeName = (String) node->attributes().get("box");
        if (boxTypeName.empty())
        {
            boxTypeName = (String) node->attributes().get("frame");
        }
        if (!boxTypeName.empty())
        {
            auto btor = CLayoutManager::boxTypeNames().find(boxTypeName);
            if (btor != CLayoutManager::boxTypeNames().end())
            {
                m_widget->box(btor->second);
            }
        }

        if (node->attributes().get("visible", "true") != "true")
        {
            m_widget->hide();
        }
        else
        {
            m_widget->show();
        }
        if (node->attributes().get("enable", "true") != "true")
        {
            m_widget->deactivate();
        }
        else
        {
            m_widget->activate();
        }
    }
    if ((int) xmlMode & (int) CLayoutXMLmode::DATA)
    {
        auto* control = dynamic_cast<CControl*>(m_widget);
        if (control)
        {
            control->load(node, CLayoutXMLmode::DATA);
        }
    }
}

void CLayoutClient::save(const xdoc::SNode& node, CLayoutXMLmode xmlMode) const
{
    String className = "widget";
    auto* layoutClient = dynamic_cast<CLayoutClient*>(m_widget);
    if (layoutClient)
    {
        className = layoutClient->className();
    }
    node->name(className);

    if ((int) xmlMode & (int) CLayoutXMLmode::LAYOUT)
    {
        if (!m_name.empty())
        {
            node->attributes().set("name", name());
        }
        if (!m_label.empty())
        {
            node->attributes().set("label", label());
        }
        if (!m_widget->visible())
        {
            node->attributes().set("visible", m_widget->visible() ? "true" : "false");
        }
        if (!m_widget->active())
        {
            node->attributes().set("enable", m_widget->active() ? "true" : "false");
        }

        String layoutAlignStr;
        switch (m_layoutAlign)
        {
            case CLayoutAlign::TOP:
                layoutAlignStr = "top";
                break;
            case CLayoutAlign::BOTTOM:
                layoutAlignStr = "bottom";
                break;
            case CLayoutAlign::LEFT:
                layoutAlignStr = "left";
                break;
            case CLayoutAlign::RIGHT:
                layoutAlignStr = "right";
                break;
            case CLayoutAlign::CLIENT:
                layoutAlignStr = "client";
                break;
            default:
                break;
        }

        if (layoutAlignStr.empty())
        {
            node->attributes().set("x", to_string(m_widget->x()));
            node->attributes().set("y", to_string(m_widget->y()));
            node->attributes().set("w", to_string(m_widget->w()));
            node->attributes().set("h", to_string(m_widget->h()));
        }
        else
        {
            node->attributes().set("layout_align", layoutAlignStr);
            node->attributes().set("layout_size", to_string(layoutSize()));
        }
    }
    else
    {
        if (!m_name.empty())
        {
            node->attributes().set("name", name());
        }
        else if (!m_label.empty())
        {
            node->attributes().set("label", label());
        }
    }
    if ((int) xmlMode & (int) CLayoutXMLmode::DATA)
    {
        auto* control = dynamic_cast<CControl*>(m_widget);
        if (control != nullptr)
        {
            control->save(node, CLayoutXMLmode::DATA);
        }
    }
}

int CLayoutClient::lastPreferredH() const
{
    return m_lastPreferredH;
}

int CLayoutClient::lastPreferredW() const
{
    return m_lastPreferredW;
}

void CLayoutClient::lastPreferredH(int height)
{
    m_lastPreferredH = height;
}

void CLayoutClient::lastPreferredW(int width)
{
    m_lastPreferredW = width;
}
