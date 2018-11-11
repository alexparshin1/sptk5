/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CDropDownBox.cpp - description                         ║
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
#include <sptk5/gui/CDropDownBox.h>

using namespace sptk;

//===========================================================================
void CDBDropDownListView::item_clicked(int /*mouse_btn*/)
{
    auto parentWindow = (CDBDropDownList*) window();
    parentWindow->m_clicked = 1;
    parentWindow->hide();
}

void CDBDropDownListView::onExit()
{
    auto wnd = (CDBDropDownList*) window();
    wnd->m_clicked = -1;
    wnd->hide();
}

//===========================================================================
CDBDropDownList::CDBDropDownList(int width, int height, const char* label)
: CPopupWindow(width, height, label)
{
    layoutSpacing(0);
    begin();
    listView = new CDBDropDownListView("", 10, SP_ALIGN_NONE);
    end();
    listView->showGrid(false);
    listView->box(FL_FLAT_BOX);
    listView->headerHeight(0);
}

void CDBDropDownList::resize(int newX, int newY, int newW, int newH)
{
    listView->resize(Fl::box_dx(box()), Fl::box_dy(box()), newW - Fl::box_dw(box()), newH - Fl::box_dh(box()));
    CWindow::resize(newX, newY, newW, newH);
}

bool CDBDropDownList::preferredSize(int& w, int& h)
{
    // required height of the drop-down box
    h = listView->fullHeight() + CListView::m_scrollbarWidth + Fl::box_dh(box());
    w = listView->fullWidth() + CListView::m_scrollbarWidth + Fl::box_dw(box());
    return true;
}

int CDBDropDownList::handle(int event)
{
    int rc = CPopupWindow::handle(event);

    if (rc) return rc;

    return listView->handle(event);
}
