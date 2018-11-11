/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       directory_ds.cpp - description                         ║
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

#include <cstdio>

#include <sptk5/cgui>

using namespace std;
using namespace sptk;

CListView* filesListView;
DirectoryDS* directoryDS;
CInput* directoryInput;

void exit_cb(Fl_Widget* w, void*)
{
    w->window()->hide();
}

void go_cb(Fl_Widget*, void*)
{
    directoryDS->directory(directoryInput->data().asString());
    filesListView->fill(*directoryDS);
}

void list_view_cb(Fl_Widget* w, void*)
{
    auto listView = (CListView*) w;
    if (listView->eventType() == CE_MOUSE_DOUBLE_CLICK) {
        CPackedStrings& row = *listView->selectedRow();
        if (strncmp(row[3], "Directory", 9) == 0) {
            std::string fullPath = directoryDS->directory() + row[1];
            directoryDS->directory(fullPath);
            directoryInput->data(fullPath);
            listView->fill(*directoryDS);
        }
    }
}

int main(int argc, char* argv[])
{
    try {
        // Initialize themes
        CThemes themes;

        CWindow mainWindow(500, 400, "CDirectoryDS Test");

        CGroup agroup;
        directoryInput = new CInput("Local directory:", 10, SP_ALIGN_CLIENT);
        directoryInput->labelWidth(120);
#ifdef _WIN32
        directoryInput->data("C:\\");
#else
        directoryInput->data("/");
#endif
        CButton goButton(SP_EXEC_BUTTON, SP_ALIGN_RIGHT, "Chdir");
        goButton.callback(go_cb);
        agroup.end();

        CGroup agroup2("", 10, SP_ALIGN_BOTTOM);
        CButton exitButton(SP_EXIT_BUTTON, SP_ALIGN_RIGHT);
        exitButton.callback(exit_cb);
        agroup2.end();

        filesListView = new CListView("", 10, SP_ALIGN_CLIENT);
        filesListView->callback(list_view_cb);
        directoryDS = new DirectoryDS();

        mainWindow.end();
        mainWindow.resizable(mainWindow);
        mainWindow.show(argc, argv);

        goButton.do_callback();

        int rc = Fl::run();

        delete directoryDS;

        return rc;
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
