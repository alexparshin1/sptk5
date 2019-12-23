/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       unique_instance.cpp - description                      ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

// This example shows how to create "unique instance" application.
// Such application may only have one process running simultaneously on the same computer.

#include <sptk5/cutils>
#include <sptk5/UniqueInstance.h>

using namespace std;
using namespace sptk;

int main()
{
   // Define the unique-instance name
   UniqueInstance instance("mytest");

   if (instance.isUnique()) {
      COUT("-------- Test for UNIQUE APPLICATION INSTANCE ------------" << endl)
      COUT("To test it, try to start another copy of application while" << endl)
      COUT("the first copy is still running. Type 'end' to exit test." << endl)

      // Unique instance, wait here
      char buffer[128];
      do {
         cin.getline(buffer, sizeof(buffer) - 2);
         if (strstr(buffer, "end") != nullptr)
            break;
      } while (strstr(buffer, "end") == nullptr);
   } else
      COUT("Another instance of the program is running. Exiting." << endl)

   return 0;
}
