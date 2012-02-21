/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          datetime.cpp  -  description
                             -------------------
    begin                : Mon Sep 28, 2003
    copyright            : (C) 1999-2008 by Alexey Parshin
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

#include <stdio.h>
#include <sptk5/CDateTime.h>

using namespace sptk;

int main(int argc, char *argv[]) {

   puts("Define the date as 2003/09/28, and print the date components:");
   
   CDateTime   dt(2003, 9, 28);
   printf("Year:  %i\n", dt.year());
   printf("Month: %i, %s\n", dt.month(), dt.monthName().c_str());
   printf("Day:   %i, %s\n", dt.day(), dt.dayOfWeekName().c_str());
   printf("Date:  %s\n", dt.dateString().c_str());
   printf("Time:  %s\n", dt.timeString().c_str());
   
   puts("\nGet the date and time from the system, and print the date components:");
   dt = CDateTime::Now();
   
   /// Printing the date components:
   printf("Year:  %i\n", dt.year());
   printf("Month: %i, %s\n", dt.month(), dt.monthName().c_str());
   printf("Day:   %i, %s\n", dt.day(), dt.dayOfWeekName().c_str());
   printf("Date:  %s\n", dt.dateString().c_str());
   printf("Time:  %s\n", dt.timeString().c_str());

   CDateTime	dd(2000,1,1);
   unsigned month = 1;
   for (unsigned day = 0; day < 732; day++) {
      CDateTime nd = double(dd) + 1.0;
      if (nd.month() != dd.month()) {
        printf("%i/%i, %s %i\n", dd.day(), dd.month(), dd.monthName().c_str(),dd.year());
        printf("%i/%i, %s %i\n", nd.day(), nd.month(), nd.monthName().c_str(),nd.year());
      }
      dd = nd;
   }
   
   return 0;
}
