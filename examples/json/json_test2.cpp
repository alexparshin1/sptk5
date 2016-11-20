/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       json_test2.cpp - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 16 2013                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace sptk;

int main(int argc, char **argv)
{
    json::Document jsonDocument;

    // Get document root element
    json::Element& root = jsonDocument.root();

    // Fastest way to insert or replace element
    root.add("boolean", new json::Element(true));
    root.add("empty", new json::Element());
    root.add("string", new json::Element("test1"));

    // Convenient way to insert or replace element.
    // Recognised types: bool, integers, floats, const char*, std::string, json::ArrayData*, json::ObjectData*
    root["number"] = 124.0;
    root["number2"] = 124.0;
    root["string"] = "test";
    root["boolean"] = true;

    json::ArrayData* array1 = new json::ArrayData;
    array1->add(new json::Element(100.0));
    array1->add(new json::Element("101.0"));
    array1->add(new json::Element(102.0));
    root["array"] = array1;

    // Create JSON array and insert it into JSON element (root)
    // JSON element (root) takes ownership of arrayData
    json::ArrayData* arrayData = new json::ArrayData;
    arrayData->add(100.0);
    arrayData->add("101.0");
    arrayData->add(102.0);
    root.add("numbers", arrayData);

    // Create JSON object and insert it into JSON element (root)
    // JSON element (root) takes ownership of objectData
    json::ObjectData* objectData = new json::ObjectData;
    (*objectData)["colour"] = "black";
    (*objectData)["shape"] = "cube";
    root.add("boxes", objectData);

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
