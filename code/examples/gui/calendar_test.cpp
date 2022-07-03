/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       calendar_test.cpp - description                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <sptk5/cgui>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;

CBox* dateBox;

void cb_clicked(Fl_Widget* w, void*)
{
    auto* calendar = (CCalendar*) w;
    String dateString = calendar->date().dateString();
    dateBox->data(dateString);
}

int main(int argc, char* argv[])
{
    try
    {
        // Initialize themes
        CThemes themes;

        // Main window
        CWindow w(200, 240, "Calendar Demo");

        w.layoutSpacing(10);

        CCalendar calendar("", 10, CLayoutAlign::CLIENT);
        CBox box1("", 20, CLayoutAlign::BOTTOM);
        dateBox = &box1;

        calendar.callback(cb_clicked);

        w.end();
        w.show(argc, argv);
        Fl::run();

        return EXIT_SUCCESS;
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl)
        return 1;
    }
}
