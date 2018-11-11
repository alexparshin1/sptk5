/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       radio_buttons.cpp - description                        ║
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

#include <string.h>
#include <FL/Fl.H>
#include <FL/Fl_Button.H>

#include <sptk5/cgui>

using namespace std;
using namespace sptk;

CInput *i;
CRadioButtons *cb;

void button_cb(Fl_Widget *b,void *) {
   if ( strcmp(b->label(),"Get Choice") == 0)
      i->data(cb->data());
   else  cb->data(i->data());
}

int main(int argc, char *argv[]) {
   CWindow w(400,300);

   CRadioButtons cbl("RadioButtons: ");
   cbl.buttons(Strings("First,Second,Third,A very long choice label meant to occupy two rows,*",","));
   cb = &cbl;

   CInput    input("Test");
   i = &input;

   CGroup g("",10,SP_ALIGN_BOTTOM);
   CButton   btn1("Set Choice",SP_ALIGN_RIGHT);
   btn1.callback(button_cb);

   CButton   btn2("Get Choice",SP_ALIGN_RIGHT);
   btn2.callback(button_cb);

   w.show(argc,argv);

   CThemes::set("Keramic");
   w.relayout();

   Fl::run();

   return 0;
}
