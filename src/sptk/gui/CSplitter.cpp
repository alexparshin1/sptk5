/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CSplitter.cpp - description                            ║
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
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <sptk5/gui/CSplitter.h>
#include <sptk5/gui/CGroup.h>

using namespace std;
using namespace sptk;

CSplitter::CSplitter(const char* label, int layoutSize, CLayoutAlign layoutAlign)
        : CBox(label, layoutSize, layoutAlign)
{
    m_chainedWidget = nullptr;
    m_chainedWidgetLayout = nullptr;
    m_dragging = false;
    box(FL_THIN_UP_BOX);
}

CLayoutClient* CSplitter::creator(xml::Node* node)
{
    auto widget = new CSplitter("", 10, SP_ALIGN_TOP);
    widget->load(node, LXM_LAYOUTDATA);
    return widget;
}

int CSplitter::handle(int event)
{
    switch (event) {
        case FL_ENTER:
            switch (m_layoutAlign) {
                case SP_ALIGN_TOP:
                case SP_ALIGN_BOTTOM:
                    fl_cursor(FL_CURSOR_NS);
                    break;
                case SP_ALIGN_RIGHT:
                case SP_ALIGN_LEFT:
                    fl_cursor(FL_CURSOR_WE);
                    break;
                default:
                    break;
            }
            break;

        case FL_LEAVE:
            fl_cursor(FL_CURSOR_ARROW);
            break;

        case FL_PUSH:
            m_lastDragX = Fl::event_x();
            m_lastDragY = Fl::event_y();
            m_dragging = true;
            findChainedControl();
            return true;

        case FL_RELEASE:
            m_dragging = false;
            return true;

        case FL_DRAG:
            if (m_chainedWidget) {
                //puts("DRAGGING");
                int dx = Fl::event_x() - m_lastDragX;
                int dy = Fl::event_y() - m_lastDragY;
                m_lastDragX = Fl::event_x();
                m_lastDragY = Fl::event_y();
                int newW = m_chainedWidget->w();
                int newH = m_chainedWidget->h();
                switch (m_layoutAlign) {
                    case SP_ALIGN_TOP:
                        newH += dy;
                        m_chainedWidgetLayout->preferredSize(newW, newH);
                        m_chainedWidgetLayout->layoutSize(newH);
                        break;
                    case SP_ALIGN_BOTTOM:
                        newH -= dy;
                        m_chainedWidgetLayout->preferredSize(newW, newH);
                        m_chainedWidgetLayout->layoutSize(newH);
                        break;
                    case SP_ALIGN_LEFT:
                        newW += dx;
                        m_chainedWidgetLayout->preferredSize(newW, newH);
                        m_chainedWidgetLayout->layoutSize(newW);
                        break;
                    case SP_ALIGN_RIGHT:
                        newW -= dx;
                        m_chainedWidgetLayout->preferredSize(newW, newH);
                        m_chainedWidgetLayout->layoutSize(newW);
                        break;
                    default:
                        break;
                }
                auto parentManager = dynamic_cast<CLayoutManager*>(parent());
                if (parentManager) {
                    parentManager->relayout();
                    auto parentGroup = dynamic_cast<CGroup*>(parent());
                    if (parentGroup) {
                        //puts("Group");
                        parentGroup->redraw();
                    } else {
                        //puts("Window");
                        window()->redraw();
                    }
                }
            }
            return true;

        default:
            break;
    }

    return CBox::handle(event);
}

void CSplitter::findChainedControl()
{
    m_chainedWidget = nullptr;
    try {
        if (!dynamic_cast<CLayoutManager*>(parent()))
            return;
        Fl_Group* group = parent();
        auto cnt = (unsigned) group->children();
        Fl_Widget* priorWidget = nullptr;
        Fl_Widget* nextWidget = nullptr;
        CLayoutClient* priorWidgetLayout = nullptr;
        CLayoutClient* nextWidgetLayout = nullptr;
        auto index = (unsigned) -1;
        for (unsigned i = 1; i < cnt; i++) {
            if (group->child(i) == this) {
                index = i;
                break;
            }
        }
        if (index < cnt - 1) {
            try {
                priorWidget = group->child(index - 1);
                priorWidgetLayout = dynamic_cast<CLayoutClient*>(group->child(index - 1));
            }
            catch (...) {}
            try {
                nextWidget = group->child(index + 1);
                nextWidgetLayout = dynamic_cast<CLayoutClient*>(group->child(index + 1));
            }
            catch (...) {}
        }
        if (priorWidget && priorWidgetLayout && priorWidgetLayout->layoutAlign() != SP_ALIGN_CLIENT) {
            m_chainedWidget = priorWidget;
            m_chainedWidgetLayout = priorWidgetLayout;
        } else {
            m_chainedWidget = nextWidget;
            m_chainedWidgetLayout = nextWidgetLayout;
        }

        if (m_chainedWidget && m_chainedWidgetLayout) {
            if (m_chainedWidgetLayout->layoutAlign() == SP_ALIGN_NONE) {
                m_chainedWidget = nullptr;
                return;
            }
            layoutAlign(m_chainedWidgetLayout->layoutAlign());
        }
    } catch (...) {
        return;
    }
}
