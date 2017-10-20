/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonDocument.cpp - description                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 16 2013                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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
#include <sptk5/xml/XMLDocument.h>

using namespace std;
using namespace sptk;
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

void Document::parse(const string& json)
{
    if (m_root)
        delete m_root;

    m_root = new Element;

    if (json.empty())
        return;

    Parser parser;
    parser.parse(*m_root, json);
}

Document::Document(bool isObject)
{
    if (isObject)
        m_root = new Element(new ObjectData);
    else
        m_root = new Element(new ArrayData);
}

Document::Document(Document&& other)
: m_root(other.m_root)
{
    if (m_root->type() == JDT_OBJECT)
        other.m_root = new Element(new ObjectData);
    else
        other.m_root = new Element(new ArrayData);
}

Document::~Document()
{
    if (m_root)
        delete m_root;
}

void Document::load(const string& json)
{
    parse(json);
}

void Document::load(const char* json)
{
    string json_str(json);
    parse(json_str);
}

void Document::load(istream& json)
{
    stringstream    buffer;
    string          row;
    for (;;) {
        getline(json, row);
        if (json.eof()) {
            buffer << row << "\n";
            break;
        }
        if (!json.good())
            throw Exception("Error reading JSON data from stream");
        buffer << row << "\n";
    }
    load(buffer.str());
}

void Document::exportTo(std::ostream& stream, bool formatted) const
{
    m_root->exportTo(stream, formatted);
}

void Document::exportTo(Buffer& buffer, bool formatted) const
{
    stringstream stream;
    m_root->exportTo(stream, formatted);
    buffer.set(stream.str());
}

void Document::exportTo(sptk::XMLDocument& document, const string& rootNodeName) const
{
    m_root->exportTo(rootNodeName, document);
}

Element& Document::root()
{
    return *m_root;
}

const Element& Document::root() const
{
    return *m_root;
}

/**
 * Optimize arrays
 * Walks through the JSON document, and convert objects that contain
 * only single array field, to arrays - by removing unnenecessary name.
 * @param name const std::string&, Optional field name, use any name if empty string
 */
void optimizeArrays(const std::string& name="item");
