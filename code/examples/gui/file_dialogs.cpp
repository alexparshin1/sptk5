/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       file_dialogs.cpp - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

void exit_cb(Fl_Widget* w, void*)
{
    w->window()->hide();
}

void file_open_dialog_cb(Fl_Widget*, void*)
{
    CFileOpenDialog dialog;
    dialog.directory(".");
    dialog.addPattern("C++ Files", "*.cpp;*.cxx");
    dialog.addPattern("VC++ Projects", "*.dsp");
    dialog.fileName("file_dialog.cpp");
    if (dialog.execute())
    {
        spInformation("Selected file:\n" + dialog.directory() + dialog.fileName());
    }
}

void file_save_dialog_cb(Fl_Widget*, void*)
{
    CFileSaveDialog dialog;
    dialog.directory(".");
    dialog.addPattern("C++ Files", "*.cpp;*.cxx");
    dialog.addPattern("VC++ Projects", "*.dsp");
    // Optionally, you can pre-select the file name with dialog.fileName("file_dialog.cpp");
    if (dialog.execute())
    {
        spInformation("Selected file:\n" + dialog.directory() + dialog.fileName());
    }
}

void dir_open_dialog_cb(Fl_Widget*, void*)
{
    CDirOpenDialog dialog;
    dialog.directory(".");
    if (dialog.execute())
    {
        spInformation("Selected directory:\n" + dialog.directory());
    }
}

int main(int argc, char* argv[])
{
    try
    {
        // Initialize themes
        CThemes themes;

        CWindow w(200, 150);

        CButton b1(CButtonKind::OPEN_BUTTON, CLayoutAlign::TOP,
                   "Open File Dialog");
        b1.callback(file_open_dialog_cb);

        CButton b2(CButtonKind::SAVE_BUTTON, CLayoutAlign::TOP,
                   "Save File Dialog");
        b2.callback(file_save_dialog_cb);

        CButton b3(CButtonKind::BROWSE_BUTTON, CLayoutAlign::TOP,
                   "Open Directory Dialog");
        b3.callback(dir_open_dialog_cb);

        CButton b4(CButtonKind::EXIT_BUTTON, CLayoutAlign::BOTTOM);
        b4.callback(exit_cb);

        w.end();
        w.show(argc, argv);

        return Fl::run();
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl)
        return 1;
    }
}
