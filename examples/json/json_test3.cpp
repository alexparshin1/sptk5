/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       json_test1.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 16 2013                                   ║
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

#include <sptk5/json/JsonDocument.h>

#include <fstream>

using namespace std;
using namespace sptk;

int main(int argc, char **argv)
{
    try {
        json::Document jsonDocument;

        ifstream file("test.data/test2.json");
        jsonDocument.load(file);
        file.close();

        json::ElementSet selected;
        jsonDocument.root().exportTo(cout);
        jsonDocument.root().select(selected, "//firstName");

        cout << endl << endl << "Selected " << selected.size() << " element(s):" << endl << endl;

        int elementIndex = 1;
        for (json::Element* element: selected) {
            cout << "Element " << elementIndex << ":" << endl;
            element->exportTo(cout);
            cout << endl << endl;
            elementIndex++;
        }

        return 0;
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
