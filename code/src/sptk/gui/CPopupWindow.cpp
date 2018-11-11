/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CPopupWindow.cpp - description                         ║
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

#include <sptk5/gui/CPopupWindow.h>

using namespace std;
using namespace sptk;

CPopupWindow::CPopupWindow(int w, int h, const char* label)
        : CWindow(w, h, label)
{
    m_clicked = 0;
    end();
    clear_border();
    parent(nullptr);
    box(FL_THIN_UP_BOX);
}

bool CPopupWindow::showModal()
{
    set_modal();
    m_clicked = 0;
    fl_cursor(FL_CURSOR_DEFAULT);

    Fl::grab(this);
    show();
    while (shown())
        Fl::wait();
    Fl::release();
    take_focus();

    return m_clicked == 1;
}

int CPopupWindow::handle(int event)
{
    int ex = Fl::event_x();
    int ey = Fl::event_y();
    int key;

    switch (event) {

        case FL_PUSH:
            if (ex < 0 || ex > w() || ey < 0 || ey > h()) {
                m_clicked = -1;
                hide();
                return 1;
            }
            break;

        case FL_KEYBOARD:
            key = Fl::event_key();
            switch (key) {
                case FL_Escape:
                case FL_Tab:
                    m_clicked = -1;
                    hide();
                    return 1;
                case FL_Enter:
                    //combo->item.index(drop_list->row());
                    m_clicked = 1;
                    hide();
                    return 1;
                default:
                    break;
            }
            break;

        default:
            break;
    }

    if (m_clicked) {
        hide();
        return 1;
    }

    return 0;
}
