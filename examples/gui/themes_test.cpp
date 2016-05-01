/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                               DEMO PROGRAMS SET
                          themes_test.cpp  -  description
                             -------------------
    begin                : July 30, 2006
    copyright            : (C) 1999-2016 by Alexey S.Parshin
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

 #include <sptk5/sptk.h>

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <FL/Fl.H>
#include <FL/Fl_XPM_Image.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Tiled_Image.H>
#include <FL/Fl_Group.H>

#include <sptk5/cgui>

#include <stdio.h>
#include <stdlib.h>
#include <iostream>

using namespace std;
using namespace sptk;

void exit_cb(Fl_Widget *w, void *) {
   w->window()->hide();
}

void theme_cb(Fl_Widget *w, void *) {
   CComboBox *themesCombo = (CComboBox *)w;
   std::string themeName = themesCombo->data();

   CThemes::set(themeName);

   CWindow *window = (CWindow *)w->window();
   window->relayout();
   window->redraw();
}

int main(int argc, char **argv)
{
    // Initialize themes
    CThemes allThemes;

    Fl::visual(FL_DOUBLE | FL_INDEX);

    CXmlDoc doc;
    CWindow w(550, 450, "SPTK themes test");

    w.layoutSpacing(10);

    CGroup group1("", 10);
    group1.box(FL_DOWN_BOX);
    CButton testButton1(SP_OK_BUTTON, SP_ALIGN_LEFT);
    CButton testButton2(SP_CANCEL_BUTTON, SP_ALIGN_LEFT);
    testButton1.defaultButton(true);
    group1.end();

    CCheckButtons checkButtons("Check buttons");
    checkButtons.buttons(CStrings("button 1|button 2","|"));
    checkButtons.data("button 1");

    CRadioButtons radioButtons("Radio buttons");
    radioButtons.buttons(CStrings("button 1|button 2","|"));
    radioButtons.data("button 1");

    CInput inp1("input 1");
    CInput inp2("input 2");

    CDBListView listView("List View:", 10, SP_ALIGN_CLIENT);
    listView.columns().push_back(CColumn("column 1", VAR_INT, 70));
    listView.columns().push_back(CColumn("column 2", VAR_INT, 70));
    listView.columns().push_back(CColumn("column 3", VAR_STRING, 200));
    listView.columns().push_back(CColumn("column 4", VAR_STRING, 300));

    listView.showGrid(false);

    char buffer1[10];
    char buffer2[10];
    int maxItems = 20;
    for(int a=0; a<maxItems; a++) {
        sprintf(buffer1, "%i", a);
        sprintf(buffer2, "%i", maxItems-a);
        cpchar rowData[] = {buffer1, buffer2, "Column 2", "-----------Long column-----------"};
        CPackedStrings *ps = new CPackedStrings(4, rowData);
        listView.addRow(ps);
    }

    CProgressBar progressBar("Progress",20,SP_ALIGN_TOP);
    progressBar.data(50);

    CGroup buttonGroup("", 10, SP_ALIGN_BOTTOM);
    buttonGroup.box(FL_DOWN_BOX);

    CComboBox themesCombo("Theme:", 350, SP_ALIGN_LEFT);
    CStrings themes = CThemes::availableThemes();
    themesCombo.addRows("Theme", themes);
    themesCombo.callback(theme_cb);
    themesCombo.labelWidth(70);

    CButton* exitButton = new CButton(SP_EXIT_BUTTON, SP_ALIGN_RIGHT);
    exitButton->callback((Fl_Callback*)exit_cb);
    exitButton->defaultButton(true);

    buttonGroup.end();

    themesCombo.data("Default");
    //themesCombo.data("GTK:Brushed");

    w.end();
    w.resizable(w);

    w.show(argc, argv);

    w.relayout();

    return Fl::run();
}
