/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                               DEMO PROGRAMS SET
                          check_buttons.cpp  -  description
                             -------------------
    begin                : January 3, 2003
    copyright            : (C) 1999-2013 by Alexey S.Parshin
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it 
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at 
   your option) any later version.

   This library is distributed in the hope that it will be useful, but 
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation, 
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#ifdef __BORLANDC__
#include <vcl.h>
#pragma hdrstop
#endif

#include <sptk5/cgui>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include <FL/Fl.H>

using namespace std;
using namespace sptk;

CInput        *i;
CCheckButtons *cb;
Fl_Box        *bx;

void button_cb(Fl_Widget *b, void *) {
   if ( strcmp(b->label(), "Get Choices") == 0)
      i->data(cb->data());
   else  cb->data(i->data());
}

int main(int argc, char *argv[])
{
    // Initialize themes
    CThemes themes;

    CWindow w(400, 250);
   
   CCheckButtons cbl("Check Boxes: ");
   cbl.buttons(CStrings("first,second,third,a very long choice text meant to occupy two rows,*", ","));
   cb = &cbl;
   
   CInput    input("Test");
   i = &input;
   
   CGroup g("", 10, SP_ALIGN_BOTTOM);
   CButton   btn1("Set Choices", SP_ALIGN_RIGHT);
   btn1.callback(button_cb);
   
   CButton   btn2("Get Choices", SP_ALIGN_RIGHT);
   btn2.callback(button_cb);
   
   w.end();
   w.show(argc, argv);
   
   CThemes::set("Keramic");
   w.relayout();
   
   Fl::run();
   
   return EXIT_SUCCESS;
}
