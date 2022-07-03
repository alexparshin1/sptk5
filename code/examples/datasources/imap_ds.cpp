/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       imap_ds.cpp - description                              ║
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

#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <sptk5/cgui>
#include <sptk5/cutils>
#include <sptk5/cnet>

using namespace std;
using namespace sptk;

CListView* filesListView;
ImapDS* imapDS;
CInput* imapServer;
CInput* imapUser;
CPasswordInput* imapPassword;
CInput* imapMailBox;

void exit_cb(Fl_Widget* w, void*)
{
    w->window()->hide();
}

void go_cb(Fl_Widget*, void*)
{
    imapDS->host(Host(imapServer->data().asString()));
    imapDS->user(imapUser->data().asString());
    imapDS->password(imapPassword->data().asString());
    imapDS->folder(imapMailBox->data().asString());
    try
    {
        filesListView->fill(*imapDS);
    }
    catch (const Exception& e)
    {
        fl_alert("%s", e.what());
    }
}

int main(int argc, char* argv[])
{
    try
    {
        // Initialize themes
        CThemes themes;

        CWindow mainWindow(600, 400, "CImapDS Test");

        CGroup agroup1;
        imapServer = new CInput("Host:", 100, CLayoutAlign::LEFT);
        imapServer->labelWidth(40);
        imapUser = new CInput("User:", 100, CLayoutAlign::LEFT);
        imapUser->labelWidth(40);
        imapPassword = new CPasswordInput("Password:", 120, CLayoutAlign::LEFT);
        imapPassword->labelWidth(70);
        imapMailBox = new CInput("Mail Box:", 130, CLayoutAlign::LEFT);
        imapMailBox->labelWidth(70);
        imapMailBox->data("Inbox");
        CButton goButton(CButtonKind::EXEC_BUTTON, CLayoutAlign::RIGHT,
                         "Messages");
        goButton.callback(go_cb);
        agroup1.end();

        CGroup agroup2("", 10, CLayoutAlign::BOTTOM);
        CButton exitButton(CButtonKind::EXIT_BUTTON, CLayoutAlign::RIGHT);
        exitButton.callback(exit_cb);
        agroup2.end();

        filesListView = new CListView("", 10, CLayoutAlign::CLIENT);
        imapDS = new ImapDS();

        mainWindow.end();
        mainWindow.resizable(mainWindow);
        mainWindow.show(argc, argv);

        int rc = Fl::run();

        delete imapDS;

        return rc;
    }
    catch (const Exception& e)
    {
        CERR(e.what() << endl)
        return 1;
    }
}
