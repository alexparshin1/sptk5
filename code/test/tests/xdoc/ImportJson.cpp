/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include <gtest/gtest.h>
#include <sptk5/Printer.h>
#include <sptk5/xdoc/Document.h>
#include <sptk5/xdoc/ExportJSON.h>

using namespace std;
using namespace sptk;
using namespace xdoc;

static const String testJson(
    R"({"name":"John","age":33,"temperature":33.6,"timestamp":1519005758000,)"
    R"("skills":["C++","Java","Motorbike"],)"
    R"("location":null,)"
    R"("description":"Title: \"Mouse\"\r\nPosition:\t\fManager/Janitor\b",)"
    R"("value":"\\0x05",)"
    R"("title":"\"Mouse\"",)"
    R"("name":"Юстас",)"
    R"("address":{"married":true,"employed":false}})");

static const String testFormattedJson(R"({
  "name": "John",
  "age": 33,
  "temperature": 33.6,
  "timestamp": 1519005758000,
  "skills": [
    "C++",
    "Java",
    "Motorbike"
  ],
  "location": null,
  "description": "Title: \"Mouse\"\r\nPosition:\t\fManager/Janitor\b",
  "value": "\\0x05",
  "title": "\"Mouse\"",
  "name": "Юстас",
  "address": {
    "married": true,
    "employed": false
  }
})");

TEST(SPTK_XDocument, formatJSON)
{
    Buffer input(testJson);
    xdoc::Document document;
    const auto& root = document.root();
    Node::importJson(root, input);

    Buffer output;
    ExportJSON::exportToJSON(root.get(), output, false);

    EXPECT_STREQ(testJson.c_str(), output.c_str());

    ExportJSON::exportToJSON(root.get(), output, true);
    EXPECT_STREQ(testFormattedJson.c_str(), output.c_str());
}

TEST(SPTK_XDocument, importJsonExceptions)
{
    Buffer input("<?xml?>");
    xdoc::Document document;
    const auto& root = document.root();
    EXPECT_THROW(Node::importJson(root, input), Exception);
}
