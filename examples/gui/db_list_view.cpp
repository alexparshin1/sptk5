/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       db_list_view.cpp - description                         ║
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

#include <FL/Fl.H>
#include <iostream>
#include <sptk5/cgui>
#include <sptk5/cdatabase>
#include <sptk5/db/CODBCConnection.h>

using namespace std;
using namespace sptk;

//#include <direct.h>
#include <stdlib.h>
#include <stdio.h>

void theme_cb(Fl_Widget *w, void *) {
   try {
      CComboBox *themesCombo = (CComboBox *)w;
      std::string themeName = themesCombo->data();
      if (themesCombo->eventType() == CE_DATA_CHANGED) {
         CThemes::set(themeName);
         CWindow *window = (CWindow *)w->window();
         window->relayout();
         window->redraw();
      }
   } catch (exception& e) {
      spError(e.what());
   }
}

int main(int argc, char **argv) {
   // If you want to test the database abilities of the data controls
   // you have to setup the ODBC database connection.
   // Typical connect string is something like: "DSN=odbc_demo;UID=user;PWD=password".
   // If UID or PWD are omitted they are read from the datasource settings.

   CODBCConnection    db("DSN=odbc_demo");
   CWindow          w(500, 400, "Test for List View Dataset");

   try {
      db.open();

      // Start the transaction in hope that your database supports it.
      // If it doesn't - comment out next two lines.
      CTransaction transaction(db);
      transaction.begin();

      // Create a table
      CQuery  query(&db, "CREATE TABLE demo_table(id int,name varchar(20),position varchar(20),city varchar(20))");
      query.exec();

      // Query to insert test data into the table
      query.sql("INSERT INTO demo_table(id,name,position,city) VALUES (:id,:name,:position,:city)");

      query.param("id") = 1;
      query.param("name") = "Donald Duck";
      query.param("position") = "President";
      query.param("city") = "New Angeles";
      query.exec();

      query.param("id") = 2;
      query.param("name") = "Rocky";
      query.param("position") = "Vice President";
      query.param("city") = "Los Poganos";
      query.exec();

      query.param("id") = 3;
      query.param("name") = "Mickey Mouse";
      query.param("position") = "Sales Director";
      query.param("city") = "El Pluto";
      query.exec();

      query.param("id") = 4;
      query.param("name") = "Goofy";
      query.param("position") = "IT Director";
      query.param("city") = "Old York";
      query.exec();

      query.param("id") = 5;
      query.param("name") = "Винни Пух";
      query.param("position") = "Медведь";
      query.param("city") = "Голливуд";
      query.exec();

      // List view to show data from demo_table
      CDBListView listView("Employees:", 10, SP_ALIGN_CLIENT);
      listView.setup(&db, "SELECT name, position FROM demo_table", "id");
      listView.refreshData();

      // In case if your database doesn't support transactions - we remove the created table directly
      query.sql("DROP TABLE demo_table");
      query.exec();

      // Theme combo box
      CComboBox themesCombo("Theme", 10, SP_ALIGN_BOTTOM);
      Strings themes = CThemes::availableThemes();
      themesCombo.addRows("Theme", themes);
      themesCombo.callback(theme_cb);
      themesCombo.data("Default");

      w.end();
      w.resizable(w);
      w.show(argc, argv);
      return Fl::run();
   } catch (exception& e) {
      spError(e.what());
   }
   return 0;
}
