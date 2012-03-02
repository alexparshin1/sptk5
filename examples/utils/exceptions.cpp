/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          exceptions.cpp  -  description
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
#include <sptk5/CException.h>

int main(int argc, char *argv[]) {
  
   puts("Let's try to throw the exception and catch it:");
   
   try {
      // If something goes wrong, we can throw an exception here
      throw sptk::CException("Error in something", __FILE__, __LINE__, "The full description is here.");
   } catch (std::exception& e) {
      puts(e.what());
   }
   
   return 0;
}
