/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                               DEMO PROGRAMS SET
                          calendar_test.cpp  -  description
                             -------------------
    begin                : October 6, 2003
    copyright            : (C) 2003-2012 by Alexey S.Parshin
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

#include <FL/Fl.H>
#include <sptk5/cgui>

using namespace std;
using namespace sptk;

CBox *dateBox;

void cb_clicked(Fl_Widget *w, void *)
{
    CCalendar *calendar = (CCalendar *) w;
    string dateString = calendar->date().dateString();
    dateBox->data(dateString);
}

int main(int argc, char *argv[])
{
    CWindow w(200, 240, "Calendar Demo");

    w.layoutSpacing(10);

    CCalendar calendar("", 10, SP_ALIGN_CLIENT);
    CBox box1("", 20, SP_ALIGN_BOTTOM);
    dateBox = &box1;

    calendar.callback(cb_clicked);

    w.end();
    w.show(argc, argv);
    Fl::run();

    return EXIT_SUCCESS;
}
