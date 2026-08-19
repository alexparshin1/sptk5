/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include <sptk5/gui/CPopupWindow.h>

#include <FL/Fl.H>
#include <FL/fl_draw.H>

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
    {
        Fl::wait();
    }
    Fl::release();
    take_focus();

    return m_clicked == 1;
}

int CPopupWindow::handle(int event)
{
    int ex = Fl::event_x();
    int ey = Fl::event_y();
    int key = 0;

    switch (event)
    {

        case FL_PUSH:
            if (ex < 0 || ex > w() || ey < 0 || ey > h())
            {
                m_clicked = -1;
                hide();
                return 1;
            }
            break;

        case FL_KEYBOARD:
            key = Fl::event_key();
            switch (key)
            {
                case FL_Escape:
                case FL_Tab:
                    m_clicked = -1;
                    hide();
                    return 1;
                case FL_Enter:
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

    if (m_clicked)
    {
        hide();
        return 1;
    }

    return 0;
}
