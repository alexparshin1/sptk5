/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonDocument.cpp - description                         ║
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
#include <sptk5/json/JsonParser.h>
#include <sstream>

using namespace std;
using namespace sptk::json;

void Document::clear()
{
    json::Type elementType = m_root->type();
    if (m_root) {
        delete m_root;
        if (elementType == JDT_ARRAY)
            m_root = new Element((ArrayData*)NULL);
        else
            m_root = new Element((ObjectData*)NULL);
    }
}

void Document::parse(const string& json) throw(Exception)
{
    if (m_root) {
        delete m_root;
        m_root = new Element;
    }
    
    Parser parser;
    parser.parse(*m_root, json);
}

Document::Document(bool isObject)
{
    if (isObject) {
        ObjectData emptyObject;
        m_root = new Element(emptyObject);
    }
    else {
        ArrayData emptyArray;
        m_root = new Element(emptyArray);
    }
}

Document::~Document()
{
    if (m_root)
        delete m_root;
}

void Document::load(const string& json) throw(Exception)
{
    parse(json);
}

void Document::load(const char* json) throw(Exception)
{
    string json_str(json);
    parse(json_str);
}

void Document::load(istream& json) throw(Exception)
{
    stringstream    buffer;
    string          row;
    while (!json.eof()) {
        getline(json, row);
        buffer << row << "\n";
    }
    load(buffer.str());
}

Element& Document::root()
{
    return *m_root;
}
