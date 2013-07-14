/***************************************************************************
                          unique_test.cpp  -  description
                             -------------------
    begin                : June 6 2002
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

// This example shows how to create "unique instance" application.
// Such application may only have one process running simultaneously on the same computer.

#include <stdio.h>
#include <string.h>

#include <sptk5/CUniqueInstance.h>

using namespace sptk;

int main() {
   char buffer[1024];
   
   // Define the unique-instance name
   CUniqueInstance instance("mytest");
   
   if (instance.isUnique()) {
      puts("-------- Test for UNIQUE APPLICATION INSTANCE ------------");
      puts("To test it, try to start another copy of application while");
      puts("the first copy is still running. Type 'end' to exit test.");
      
      // Unique instance, wait here
      while (strcmp(buffer, "end") != 0) {
         scanf("%s", buffer);
      }
   } else {
      puts("Another instance of the program is running. Exiting.");
   }
   
   return 0;
}
