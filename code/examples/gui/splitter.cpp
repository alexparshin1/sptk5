/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       splitter.cpp - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#include <FL/Fl.H>
#include <sptk5/cutils>
#include <sptk5/cgui>

using namespace std;
using namespace sptk;

void exit_cb(Fl_Widget* w, void*)
{
    w->window()->hide();
}

int main(int argc, char* argv[])
{
    try {
        // Initialize themes
        CThemes themes;

        // CWindow supports auto-layout
        CWindow w(600, 400, "CSplitter test");
        w.resizable(w);
        w.layoutSpacing(4);

        auto* sb = new CScroll("", 100, SP_ALIGN_RIGHT);
        new CBox("This is a box", 100, SP_ALIGN_CLIENT);
        sb->end();

        CSplitter splitter("", 3, SP_ALIGN_RIGHT);

        CListView listView2("List View 2:", 10, SP_ALIGN_CLIENT);
        listView2.columns().push_back(CColumn("column 1", VAR_INT, 70));
        listView2.columns().push_back(CColumn("column 2", VAR_INT, 70));
        listView2.columns().push_back(CColumn("column 3", VAR_STRING, 200));
        listView2.columns().push_back(CColumn("column 4", VAR_STRING));

        // Add 10 items
        char buffer1[20];
        char buffer2[20];
        for (int a = 0; a < 10; a++) {
            snprintf(buffer1, sizeof(buffer1) - 1, "%i", a);
            snprintf(buffer2, sizeof(buffer2) - 1, "%i", 100000 - a);
            cpchar rowData[] = {buffer1, buffer2, "Column 2", "-----------Long column-----------"};
            auto* ps = new CPackedStrings(4, rowData);
            listView2.addRow(ps);
        }

        // That group keeps togeteher the buttons. These
        // buttons use the default alignment for buttons - 
        // SP_ALIGN_RIGHT, and the text/icon defined by the 
        // button kind.
        CGroup buttonGroup("", 10, SP_ALIGN_BOTTOM);
        buttonGroup.color(FL_LIGHT1);

        CButton okButton(SP_EXIT_BUTTON);
        okButton.callback(exit_cb);

        buttonGroup.end();

        w.end();
        w.show(argc, argv);

        return Fl::run();
    }
    catch (const Exception& e) {
        CERR(e.what() << endl);
        return 1;
    }
}
