/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       cscroll_test.cpp - description                         ║
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

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <sptk5/cgui>

using namespace std;
using namespace sptk;

void exit_cb(Fl_Widget* w, void*)
{
    w->window()->hide();
}

int main()
{
    try {
        // Initialize themes
        CThemes themes;

        // CWindow supports auto-layout
        CWindow w(250, 150, "CScroll test");
        w.resizable(w);

        CScroll sc("", 20, SP_ALIGN_CLIENT);
        sc.color(w.color());
        sc.box(FL_DOWN_BOX);
        sc.layoutSpacing(0);

        CGroup gr("", 20, SP_ALIGN_CLIENT);

        // CCheckButtons figures out the size from the list
        // of choices. The default alignment is SP_ALIGN_TOP,
        // and you can change it in ctor
        CCheckButtons cbl("Check Buttons: ");
        cbl.buttons(Strings("first,second,third,*", ","));
        CRadioButtons rbl("Radio Buttons: ");
        rbl.buttons(Strings("first,second,third,*", ","));

        // CListView is more flexible, than CCheckButtons or
        // CRadioButtons, it's vertical size is defined with
        // the layoutSize parameter in ctor as 150.
        // SP_ALIGN_CLIENT allows that widget to occupy all
        // the space left after all the other widgets are
        // put in place
        CListView listView("List View:", 10, SP_ALIGN_CLIENT);
        listView.columns().push_back(CColumn("column 1", VAR_INT, 70));
        listView.columns().push_back(CColumn("column 2", VAR_INT, 70));
        listView.columns().push_back(CColumn("column 3", VAR_STRING, 200));
        listView.columns().push_back(CColumn("column 4", VAR_STRING));

        // Add 10 items
        char buffer1[10];
        char buffer2[10];
        for (int a = 0; a < 10; a++) {
            sprintf(buffer1, "%i", a);
            sprintf(buffer2, "%i", 100000 - a);
            cpchar rowData[] = {buffer1, buffer2, "Column 2", "-----------Long column-----------"};
            auto ps = new CPackedStrings(4, rowData);
            listView.addRow(ps);
        }

        sc.end();

        // That group keeps togeteher the buttons. These
        // buttons use the default alignment for buttons -
        // SP_ALIGN_RIGHT, and the text/icon defined by the
        // button kind.
        CGroup buttonGroup("", 10, SP_ALIGN_BOTTOM);
        //buttonGroup.box(FL_THIN_DOWN_FRAME);
        CButton exitButton(SP_EXIT_BUTTON);
        exitButton.callback(exit_cb);
        buttonGroup.end();

        w.end();

        CThemes::set("Blue");

        w.show();

        return Fl::run();
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
