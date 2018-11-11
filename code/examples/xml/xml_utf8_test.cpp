/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       xml_utf8_test.cpp - description                        ║
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

#include <sptk5/sptk.h>
#include <sptk5/cxml>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <iomanip>

using namespace std;
using namespace sptk;

int main(int argc,char *argv[]) {
    try {
        cout << "The XML UTF8 test started." << endl << endl;

        CBuffer buf;
        XMLDoc doc;
        buf.loadFromFile("test_utf8.xml");
        doc.load(buf);

        buf.reset();
        doc.save(buf);
        buf.saveToFile("test_utf8_out.xml");

        cout << buf.data() << endl;

        cout << "The XML UTF8 test completed" << endl;

    } catch (std::exception& e) {
        cout << e.what() << endl;
        return 1;
    }
    return 0;
}
