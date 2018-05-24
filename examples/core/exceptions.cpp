/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       exceptions.cpp - description                           ║
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

#include <iostream>
#include <sptk5/Exception.h>
#include <sptk5/Buffer.h>

using namespace std;
using namespace sptk;

int main()
{
    cout << "Let's try to throw the exception and catch it:" << endl;

    try {
        // If something goes wrong, we can throw an exception here
        throw Exception("Error in something", __FILE__, __LINE__, "The full description is here.");
    } catch (exception& e) {
        cerr << "Caught exception: " << e.what() << endl;
    }

    cout << endl << "Now let's try to load non-existing file and catch the exception:" << endl;

    try {
        Buffer buffer;
        buffer.loadFromFile("/this/file/does/not/exist");
    } catch (exception& e) {
        cerr << "Caught exception: " << e.what() << endl;
    }

    return 0;
}
