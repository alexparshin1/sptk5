/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonDocument.cpp - description                         ║
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
#include <sptk5/json/JsonParser.h>

using namespace std;
using namespace sptk;
using namespace sptk::json;

void Document::clear()
{
    json::Type elementType = JDT_NULL;
    if (m_root != nullptr) {
        elementType = m_root->type();
        delete m_root;
    }

    if (elementType == JDT_ARRAY)
        m_root = new Element(this, (ArrayData*)nullptr);
    else
        m_root = new Element(this, (ObjectData*)nullptr);
}

void Document::parse(const string& json)
{
    delete m_root;

    m_root = new Element(this);

    if (json.empty())
        return;

    Parser parser;
    parser.parse(*m_root, json);
}

Document::Document(bool isObject)
: m_emptyElement(this, "")
{
    if (isObject)
        m_root = new Element(this, new ObjectData(this));
    else
        m_root = new Element(this, new ArrayData(this));
}

Document::Document(Document&& other) noexcept
: m_root(other.m_root), m_emptyElement(this, "")
{
    if (m_root->type() == JDT_OBJECT)
        other.m_root = new Element(this, new ObjectData(this));
    else
        other.m_root = new Element(this, new ArrayData(this));
}

Document::~Document()
{
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

void Document::exportTo(xml::Document& document, const string& rootNodeName) const
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

#if USE_GTEST
#include <gtest/gtest.h>

const char* testJSON =
        R"({ "name": "John", "age": 33, "temperature": 33.6, "timestamp": 1519005758000 )"
        R"("skills": [ "C++", "Java", "Motorbike" ],)"
        R"("address": { "married": true, "employed": false } })";

void verifyDocument(json::Document& document)
{
    json::Element& root = document.root();
    EXPECT_STREQ("John", root.getString("name").c_str());
    EXPECT_EQ(33, (int) root.getNumber("age"));
    EXPECT_DOUBLE_EQ(33.6, root.getNumber("temperature"));
	EXPECT_DOUBLE_EQ(1519005758000, root.getNumber("timestamp"));
	EXPECT_STREQ("1519005758000", root.getString("timestamp").c_str());

    json::ArrayData& arrayData = root.getArray("skills");
    Strings skills;
    for (auto& skill: arrayData) {
        skills.push_back(skill->getString());
    }
    EXPECT_STREQ("C++,Java,Motorbike", skills.join(",").c_str());

    json::Element* ptr = root.find("address");
    EXPECT_TRUE(ptr != nullptr);

    json::Element& address = *ptr;
    EXPECT_TRUE(address.getBoolean("married"));
    EXPECT_FALSE(address.getBoolean("employed"));
}

TEST(SPTK_JsonDocument, load)
{
    json::Document document;
    document.load(testJSON);
    verifyDocument(document);
}

TEST(SPTK_JsonDocument, add)
{
    json::Document document;
    document.load(testJSON);

    json::Element& root = document.root();

    root["int"] = 1;
    root["double"] = 2.5;
    root["string"] = "Test";
    root["bool1"] = true;
    root["bool2"] = false;

    auto arrayData = root.set_array("array");
    arrayData->push_back("C++");
    arrayData->push_back("Java");
    arrayData->push_back("Python");

    auto objectData = root.set_object("object");
    (*objectData)["height"] = 178;
    (*objectData)["weight"] = 85.5;

    EXPECT_EQ(1, (int) root.getNumber("int"));
    EXPECT_DOUBLE_EQ(2.5, root.getNumber("double"));
    EXPECT_STREQ("Test", root.getString("string").c_str());
    EXPECT_TRUE(root.getBoolean("bool1"));
    EXPECT_FALSE(root.getBoolean("bool2"));

    json::ArrayData& array = root.getArray("array");
    Strings skills;
    for (auto& skill: array)
        skills.push_back(skill->getString());
    EXPECT_STREQ("C++,Java,Python", skills.join(",").c_str());

    json::Element* object = root.find("object");
    EXPECT_TRUE(object != nullptr);
    EXPECT_EQ(178, object->getNumber("height"));
    EXPECT_DOUBLE_EQ(85.5, object->getNumber("weight"));
}

TEST(SPTK_JsonDocument, remove)
{
    json::Document document;
    document.load(testJSON);

    json::Element& root = document.root();
    root.remove("name");
    root.remove("age");
    root.remove("skills");
    root.remove("address");
    EXPECT_FALSE(root.find("name"));
    EXPECT_FALSE(root.find("age"));
    EXPECT_TRUE(root.find("temperature"));
    EXPECT_FALSE(root.find("skills"));
    EXPECT_FALSE(root.find("address"));
}

TEST(SPTK_JsonDocument, save)
{
    json::Document document;
    document.load(testJSON);

    Buffer buffer;
    document.exportTo(buffer, false);

    document.load(testJSON);
    verifyDocument(document);
}

#endif
