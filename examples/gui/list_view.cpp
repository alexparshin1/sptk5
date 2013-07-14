/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                               DEMO PROGRAMS SET
                          list_view.cpp  -  description
                             -------------------
    begin                : January 3, 2003
    copyright            : (C) 1999-2013 by Alexey S.Parshin
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

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <FL/Fl.H>

#include <sptk5/cgui>

using namespace sptk;

#define ROWS_AUTO_HEIGHT    1
#define SHOW_GRID           2
#define SHOW_STRIPES        4

CDBListView    *lView;
CCheckButton   *showGridCheckButton;
CCheckButton   *showStripesCheckButton;

void cb_cb(Fl_Widget *w, void *data) {
   CCheckButton *cb = (CCheckButton *)w;
   switch ((size_t)data) {
      case ROWS_AUTO_HEIGHT:
         printf("Row auto height %i\n", cb->value());
         lView->autoRowHeight(cb->value()!=0);
         break;
      case SHOW_GRID:
         printf("Show Grid %i\n", cb->value());
         lView->showGrid(cb->value()!=0);
         showStripesCheckButton->value(false);
         break;
      case SHOW_STRIPES:
         printf("Show Grid %i\n", cb->value());
         lView->showStripes(cb->value()!=0);
         showGridCheckButton->value(false);
         break;
   }
}

void exit_cb(Fl_Widget *w, void *data) {
   w->window()->hide();
}

int main(int argc, char **argv) {
   
   static char buffer[128];
   CWindow w(400, 350);
   
   CDBListView listView("List View:", 10, SP_ALIGN_CLIENT);
   lView = &listView;
   
   std::string operation;
   CDateTime start = CDateTime::Now();
   
   listView.columns().push_back(CColumn("column 1", VAR_INT, 70));
   listView.columns().push_back(CColumn("column 2", VAR_INT, 70));
   listView.columns().push_back(CColumn("column 3", VAR_STRING, 200));
   listView.columns().push_back(CColumn("column 4", VAR_STRING));
   
   listView.showGrid(false);
   
   char buffer1[10];
   char buffer2[10];
   int maxItems = 10000;
   for(int a=0; a<maxItems; a++) {
      sprintf(buffer1, "%i", a);
      sprintf(buffer2, "%i", maxItems-a);
      cpchar rowData[] = {buffer1, buffer2, "Column 2", "-----------Long column-----------"};
      CPackedStrings *ps = new CPackedStrings(4, rowData);
      listView.addRow(ps);
   }
   CDateTime end = CDateTime::Now();
   
   sprintf(buffer, "Filled list with 10K lines for = %0.2f sec",
           ((double)end - double(start)) * 24 * 3600
           );
   
   w.label(buffer);
   
   CGroup g1("", 20, SP_ALIGN_BOTTOM);
   
   CGroup g2("Grid Parameters", 150, SP_ALIGN_LEFT);
   CCheckButton cb1("Row auto height", 10, SP_ALIGN_BOTTOM);
   cb1.callback(cb_cb, (void *)ROWS_AUTO_HEIGHT);
   CCheckButton cb2("Show grid", 10, SP_ALIGN_BOTTOM);
   cb2.callback(cb_cb, (void *)SHOW_GRID);
   showGridCheckButton = &cb2;
   CCheckButton cb3("Show stripes", 10, SP_ALIGN_BOTTOM);
   cb3.callback(cb_cb, (void *)SHOW_STRIPES);
   showStripesCheckButton = &cb3;
   g2.end();
   
   CButton b(SP_EXIT_BUTTON, SP_ALIGN_RIGHT);
   b.callback(exit_cb);
   
   w.end();
   w.resizable(w);
   
   CThemes::set("OSX");
   w.relayout();
   w.show(argc, argv);
   
   return Fl::run();
}
