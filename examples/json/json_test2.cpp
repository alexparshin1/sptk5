/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       json_test2.cpp - description                           ║
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

using namespace std;
using namespace sptk;

int main(int argc, char **argv)
{
    try {
        json::Document jsonDocument;

        // Get document root element
        json::Element& root = jsonDocument.root();

        // Fastest way to insert or replace element
        root.set("boolean", true);
        root.set("empty");
        root.set("string", "test1");

        // Convenient way to insert or replace element.
        // Recognised types: bool, integers, floats, const char*, std::string, json::ArrayData*, json::ObjectData*
        root["number"] = 124.0;
        root["number2"] = 124.0;
        root["string"] = "test";
        root["boolean"] = true;

        auto array = root.set_object("array");
        array->push_back(100.0);
        array->push_back("101.0");
        array->push_back(102.0);

        // Create JSON object and insert it into JSON element (root)
        // JSON element (root) takes ownership of objectData
        auto objectData = root.set_object("boxes");
        (*objectData)["colour"] = "black";
        (*objectData)["shape"] = "cube";

        // Get existing JSON object and add some data to it
        json::Element& boxes = root["boxes"];
        boxes["size"] = 10;
        boxes["address"]["street"] = "17 Elm street";
        boxes["address"]["first name"] = "Freddy";
        boxes["address"]["last name"] = "Kruger";

        root.exportTo(cout, true);
        cout << endl;

        return 0;
    }
    catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
}
