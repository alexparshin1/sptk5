/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                               DEMO PROGRAMS SET
                          imap_ds.cpp  -  description
                             -------------------
    begin                :  05 July 2003
    copyright            : (C) 1999-2014 by Alexey S.Parshin
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <FL/Fl.H>
#include <FL/fl_ask.H>
#include <sptk5/cgui>
#include <sptk5/cnet>

using namespace std;
using namespace sptk;

CListView      *filesListView;
CImapDS        *imapDS;
CInput         *imapServer;
CInput         *imapUser;
CPasswordInput *imapPassword;
CInput         *imapMailBox;

void exit_cb(Fl_Widget *w,void *) {
   w->window()->hide();
}

void go_cb(Fl_Widget *w,void *) {
   imapDS->host(imapServer->data());
   imapDS->user(imapUser->data());
   imapDS->password(imapPassword->data());
   imapDS->folder(imapMailBox->data());
   try {
      filesListView->fill(*imapDS);
   }
   catch (exception& e) {
      fl_alert("%s", e.what());
   }
}

int main(int argc,char *argv[])
{
    // Initialize themes
    CThemes themes;

    CWindow mainWindow(600, 400, "CImapDS Test");

    CGroup agroup1;
    imapServer = new CInput("Host:",100,SP_ALIGN_LEFT);
    imapServer->labelWidth(40);
    imapUser = new CInput("User:",100,SP_ALIGN_LEFT);
    imapUser->labelWidth(40);
    imapPassword = new CPasswordInput("Password:",120,SP_ALIGN_LEFT);
    imapPassword->labelWidth(70);
    imapMailBox = new CInput("Mail Box:",130,SP_ALIGN_LEFT);
    imapMailBox->labelWidth(70);
    imapMailBox->data("Inbox");
    CButton goButton(SP_EXEC_BUTTON,SP_ALIGN_RIGHT,"Messages");
    goButton.callback(go_cb);
    agroup1.end();

    CGroup agroup2("",10,SP_ALIGN_BOTTOM);
    CButton exitButton(SP_EXIT_BUTTON,SP_ALIGN_RIGHT);
    exitButton.callback(exit_cb);
    agroup2.end();

    filesListView = new CListView("",10,SP_ALIGN_CLIENT);
    imapDS = new CImapDS();

    mainWindow.end();
    mainWindow.resizable(mainWindow);
    mainWindow.show(argc,argv);

    //goButton.do_callback();

    int rc = Fl::run();

    delete imapDS;

    return rc;
}
