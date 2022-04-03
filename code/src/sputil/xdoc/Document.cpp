/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/Printer.h>
#include <sptk5/StopWatch.h>
#include <sptk5/xdoc/Document.h>

using namespace std;
using namespace sptk;
using namespace sptk::xdoc;

static DataFormat autoDetectFormat(const char* data)
{
    switch (auto skip = strspn(data, "\n\r\t ");
            data[skip])
    {
        case '<':
            return DataFormat::XML;
        case '[':
        case '{':
            return DataFormat::JSON;
        default:
            break;
    }
    throw Exception("Invalid character at the data start");
}

void Document::load(const Buffer& data, bool xmlKeepFormatting) const
{
    m_root->load(autoDetectFormat(data.c_str()), data, xmlKeepFormatting);
}

void Document::load(const String& data, bool xmlKeepFormatting) const
{
    m_root->load(autoDetectFormat(data.c_str()), data, xmlKeepFormatting);
}

#ifdef USE_GTEST

const String testJSON(
    R"({ "name": "John", "age": 33, "temperature": 33.6, "timestamp": 1519005758000 )"
    R"("skills": [ "C++", "Java", "Motorbike" ],)"
    R"("location": null,)"
    R"("title": "\"Mouse\"",)"
    R"("address": { "married": true, "employed": false } })");

void verifyDocument(xdoc::Document& document)
{
    const auto& root = *document.root();
    EXPECT_STREQ("John", root.getString("name").c_str());
    EXPECT_EQ(33, (int) root.getNumber("age"));
    EXPECT_DOUBLE_EQ(33.6, root.getNumber("temperature"));
    EXPECT_STREQ("1519005758000", root.getString("timestamp").c_str());
    EXPECT_DOUBLE_EQ(1519005758000L, root.getNumber("timestamp"));

    const auto& arrayData = root.nodes("skills");
    Strings skills;
    skills.resize(arrayData.size());
    transform(arrayData.begin(), arrayData.end(), skills.begin(),
              [](const xdoc::SNode& skill) {
                  return skill->getString();
              });
    EXPECT_STREQ("C++,Java,Motorbike", skills.join(",").c_str());

    const auto ptr = root.findFirst("address");
    EXPECT_TRUE(ptr != nullptr);

    const xdoc::Element& address = *ptr;
    EXPECT_TRUE(address.getBoolean("married"));
    EXPECT_FALSE(address.getBoolean("employed"));
}

TEST(SPTK_XDocument, load)
{
    Buffer input(testJSON);
    xdoc::Document document;
    document.load(input);
    verifyDocument(document);
}

static const String testXmlDocument(
    "<xml encoding=\"utf-8\">"
    "<name>John</name>"
    "<address><city>Walhalla</city><street>17 Elm Street</street></address>"
    "</xml>");


TEST(SPTK_XDocument, clone)
{
    Buffer input(testXmlDocument);
    xdoc::Document document;
    document.load(input);

    xdoc::Document document2;
    xdoc::Node::clone(document2.root(), document.root());

    Buffer output;
    document2.exportTo(DataFormat::XML, output, false);

    EXPECT_STREQ(testXmlDocument.c_str(), output.c_str());
}

TEST(SPTK_XDocument, clone2)
{
    Buffer input(testJSON);
    xdoc::Document document;
    document.load(input);

    xdoc::Document document2;

    xdoc::Node::clone(document2.root(), document.root());

    verifyDocument(document);
}

TEST(SPTK_XDocument, add)
{
    Buffer input(testJSON);
    xdoc::Document document;
    document.load(input);

    auto& root = *document.root();

    constexpr int testInteger = 178;
    constexpr double testDouble1 = 2.5;
    constexpr double testDouble2 = 85.5;

    root.set("int", 1);
    root.set("double", testDouble1);
    root.set("string", "Test");
    root.set("bool1", true);
    root.set("bool2", false);

    const auto& arrayData = root.pushNode("array");
    arrayData->pushValue("C++");
    arrayData->pushValue("Java");
    arrayData->pushValue("Python");

    auto& objectData = *root.pushNode("object");
    objectData.set("height", testInteger);
    objectData.set("weight", testDouble2);

    EXPECT_EQ(1, (int) root.getNumber("int"));
    EXPECT_DOUBLE_EQ(testDouble1, root.getNumber("double"));
    EXPECT_STREQ("Test", root.getString("string").c_str());
    EXPECT_TRUE(root.getBoolean("bool1"));
    EXPECT_FALSE(root.getBoolean("bool2"));

    const auto& array = root.nodes("array");
    Strings skills;
    skills.resize(array.size());
    transform(array.begin(), array.end(), skills.begin(),
              [](const xdoc::SNode& skill) {
                  return skill->getString();
              });
    EXPECT_STREQ("C++,Java,Python", skills.join(",").c_str());

    const auto object = root.findFirst("object");
    EXPECT_TRUE(object != nullptr);
    EXPECT_EQ(testInteger, object->getNumber("height"));
    EXPECT_DOUBLE_EQ(testDouble2, object->getNumber("weight"));
}

TEST(SPTK_XDocument, remove)
{
    Buffer input(testJSON);
    xdoc::Document document;
    document.load(input);

    auto& root = *document.root();
    root.remove("name");
    root.remove("age");
    root.remove("skills");
    root.remove("address");
    EXPECT_FALSE(root.findFirst("name"));
    EXPECT_FALSE(root.findFirst("age"));
    EXPECT_TRUE(root.findFirst("temperature"));
    EXPECT_FALSE(root.findFirst("skills"));
    EXPECT_FALSE(root.findFirst("address"));
}

TEST(SPTK_XDocument, clear)
{
    Buffer input(testJSON);
    xdoc::Document document;
    document.load(input);

    document.root()->clear();
    const auto& root = *document.root();
    EXPECT_TRUE(root.type() == Node::Type::Object);
    EXPECT_FALSE(root.findFirst("address"));
    EXPECT_EQ(root.nodes().size(), size_t(0));
}

TEST(SPTK_XDocument, exportToBuffer)
{
    Buffer input(testJSON);
    xdoc::Document document;
    document.load(input);

    Buffer buffer;
    document.exportTo(DataFormat::JSON, buffer, false);

    document.load(input);
    verifyDocument(document);
}

TEST(SPTK_XDocument, copyCtor)
{
    Buffer input(testJSON);
    xdoc::Document document;
    document.load(input);

    xdoc::Document document2(document);

    verifyDocument(document);
    verifyDocument(document2);
}

TEST(SPTK_XDocument, moveCtor)
{
    Buffer input(testJSON);
    xdoc::Document document;
    document.load(input);

    xdoc::Document document2(move(document));

    verifyDocument(document2);
}

TEST(SPTK_XDocument, copyAssign)
{
    Buffer input(testJSON);
    xdoc::Document document;
    document.load(input);

    xdoc::Document document2;

    document2 = document;

    verifyDocument(document);
    verifyDocument(document2);
}

TEST(SPTK_XDocument, moveAssign)
{
    Buffer input(testJSON);
    xdoc::Document document;
    document.load(input);

    xdoc::Document document2;

    document2 = move(document);

    verifyDocument(document2);
}

TEST(SPTK_XDocument, truncated)
{
    xdoc::Document document;
    String truncatedJSON = testJSON.substr(0, testJSON.length() - 3);
    Buffer input(truncatedJSON);
    try
    {
        document.load(input);
        FAIL() << "Incorrect: MUST fail";
    }
    catch (const Exception& e)
    {
        SUCCEED() << "Correct: " << e.what();
    }
}

TEST(SPTK_XDocument, errors)
{
    xdoc::Document document;
    size_t errorCount = 0;

    Buffer junkTailJSON(String(testJSON) + "=");
    try
    {
        document.load(junkTailJSON);
        FAIL() << "Incorrect: MUST fail";
    }
    catch (const Exception&)
    {
        ++errorCount;
    }

    Buffer junkInsideJSON(testJSON);
    junkInsideJSON[0] = '?';
    try
    {
        document.load(junkInsideJSON);
        FAIL() << "Incorrect: MUST fail";
    }
    catch (const Exception&)
    {
        ++errorCount;
    }

    try
    {
        Buffer input(testJSON);
        document.load(input);
        auto element = document.root()->findFirst("nothing");
        if (element != nullptr)
            FAIL() << "Incorrect: MUST return null";
        const auto& root = document.root();
        element = root->findFirst("name");
        element = element->findFirst("nothing");
        if (element != nullptr)
            FAIL() << "Incorrect: MUST fail";
    }
    catch (const Exception&)
    {
        ++errorCount;
    }

    SUCCEED() << "Detected " << errorCount << " errors";
}

TEST(SPTK_XDocument, performance)
{
    constexpr int objectCount = 50000;

    xdoc::Document document;

    const auto& arrayElement = document.root()->pushNode("items");
    for (int i = 0; i < objectCount; ++i)
    {
        auto& object = *arrayElement->pushNode("", Node::Type::Object);
        object.set("id", i);
        object.set("name", String("Name " + to_string(i)));
        object.set("exists", true);

        auto& address = *object.pushNode("address", Node::Type::Object);
        address.set("number", i);
        address.set("street", String("Street " + to_string(i)));

        auto& list = *address.pushNode("list", Node::Type::Array);
        list.pushValue(1);
        list.pushValue("two");
        list.pushValue(3);
    }

    // Verify data
    const auto& arrayData = arrayElement->nodes();
    constexpr int someIndex = 100;
    const auto& object = arrayData[someIndex];
    EXPECT_FLOAT_EQ(object->getNumber("id"), 100.0);
    EXPECT_STREQ(object->getString("name").c_str(), "Name 100");

    const Node& address = *object->findFirst("address");
    EXPECT_FLOAT_EQ(address.getNumber("number"), 100.0);
    EXPECT_STREQ(address.getString("street").c_str(), "Street 100");

    const auto& list = address.nodes("list");
    EXPECT_STREQ(list[1]->getString().c_str(), "two");

    Buffer buffer;
    document.exportTo(DataFormat::JSON, buffer, true);

    StopWatch stopWatch;
    stopWatch.start();

    xdoc::Document document1;
    document1.load(buffer);

    stopWatch.stop();

    COUT("Parsed JSON document (" << objectCount << ") objects for " << stopWatch.seconds() << " seconds" << endl)
}

TEST(SPTK_XDocument, exportText)
{
    xdoc::Document document;
    auto testNode = document.root()->pushNode("test");
    auto textNode = testNode->set("#text", "ttt");

    Buffer output;
    document.exportTo(DataFormat::XML, cout, true);
}

#endif
