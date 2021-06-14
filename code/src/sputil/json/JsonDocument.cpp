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

#include <sptk5/json/JsonParser.h>
#include <sptk5/json/JsonDocument.h>
#include <sptk5/StopWatch.h>
#include <sptk5/Printer.h>

using namespace std;
using namespace sptk;
using namespace sptk::json;

void Document::clear()
{
    json::Type elementType = Type::NULL_VALUE;
    if (m_root != nullptr)
        elementType = m_root->type();

    if (elementType == Type::ARRAY) {
        auto* arrayData = new ArrayData(this);
        m_root = make_shared<Element>(this, arrayData);
    } else {
        auto* objectData = new ObjectData(this);
        m_root = make_shared<Element>(this, objectData);
    }
}

void Document::parse(const String& json)
{
    if (json.empty()) {
        auto* objectData = new ObjectData(this);
        m_root = make_shared<Element>(this, objectData);
        return;
    }

    m_root = make_shared<Element>(this);

    Parser::parse(*m_root, json);
}

Document::Document(bool isObject)
: m_root(createDocumentRoot(isObject ? Type::OBJECT : Type::ARRAY)),
  m_emptyElement(this, "")
{}

Document::Document(const Document& other)
: m_emptyElement(this, "")
{
    Buffer buffer;
    other.exportTo(buffer);
    load(buffer.c_str());
}

Document& Document::operator = (const Document& other)
{
    if (&other != this) {
        Buffer buffer;
        other.exportTo(buffer);
        load(buffer.c_str());
    }
    return *this;
}

Document::Document(Document&& other) noexcept
: m_root(move(other.m_root)), m_emptyElement(this, "")
{
    other.m_root = createDocumentRoot(m_root->type());
}

std::shared_ptr<Element> Document::createDocumentRoot(Type documentType)
{
    if (documentType == Type::OBJECT) {
        auto* objectData = new ObjectData(this);
        return make_shared<Element>(this, objectData);
    } else {
        auto* arrayData = new ArrayData(this);
        return make_shared<Element>(this, arrayData);
    }
}

Document& Document::operator = (Document&& other) noexcept
{
    m_root = move(other.m_root);
    return *this;
}

void Document::load(const String& json)
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
    streampos       pos = json.tellg();
    json.seekg (0, ios::end);

    streampos       length = json.tellg() - pos;
    json.seekg (pos, ios::beg);

    Buffer buffer((size_t)length);
    json.read(buffer.data(), length);
    buffer.bytes((size_t)length);
    load(buffer.c_str());
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

void Document::exportTo(xml::Document& document, const String& rootNodeName) const
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

Type Document::dataType(const String& data)
{
    static const RegularExpression isNumber(R"(^[+\-]?\d+(\.\d*)?(E[+\-]?\d+)?$)", "i");
    static const RegularExpression isBoolean(R"(^(true|false)$)", "i");

    Type type = Type::STRING;
    if (data.empty())
        return type;

    if (isalpha(data[0])) {
        if (isBoolean.matches(data))
            type = Type::BOOLEAN;
        else if (data == "null")
            type = Type::NULL_VALUE;
    }
    else if (isNumber.matches(data))
        type = Type::NUMBER;

    return type;
}

#if USE_GTEST

const String testJSON(
        R"({ "name": "John", "age": 33, "temperature": 33.6, "timestamp": 1519005758000 )"
        R"("skills": [ "C++", "Java", "Motorbike" ],)"
        R"("location": null,)"
        R"("title": "\"Mouse\"",)"
        R"("address": { "married": true, "employed": false } })");

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
    skills.resize(arrayData.size());
    transform(arrayData.begin(), arrayData.end(), skills.begin(),
              [](const json::Element* skill) { return skill->getString(); });
    EXPECT_STREQ("C++,Java,Motorbike", skills.join(",").c_str());

    const json::Element* ptr = root.find("address");
    EXPECT_TRUE(ptr != nullptr);

    const json::Element& address = *ptr;
    EXPECT_TRUE(address.getBoolean("married"));
    EXPECT_FALSE(address.getBoolean("employed"));
}

TEST(SPTK_JsonDocument, loadString)
{
    json::Document document;
    document.load(testJSON);
    verifyDocument(document);
}

TEST(SPTK_JsonDocument, loadStream)
{
    json::Document document;
    stringstream   data;
    data << testJSON;
    document.load(data);
    verifyDocument(document);
}

TEST(SPTK_JsonDocument, add)
{
    json::Document document;
    document.load(testJSON);

    json::Element& root = document.root();

    constexpr int testInteger = 178;
    constexpr double testDouble1 = 2.5;
    constexpr double testDouble2 = 85.5;

    root["int"] = 1;
    root["double"] = testDouble1;
    root["string"] = "Test";
    root["bool1"] = true;
    root["bool2"] = false;

    auto* arrayData = root.add_array("array");
    arrayData->push_back("C++");
    arrayData->push_back("Java");
    arrayData->push_back("Python");

    auto* objectData = root.add_object("object");
    (*objectData)["height"] = testInteger;
    (*objectData)["weight"] = testDouble2;

    EXPECT_EQ(1, (int) root.getNumber("int"));
    EXPECT_DOUBLE_EQ(testDouble1, root.getNumber("double"));
    EXPECT_STREQ("Test", root.getString("string").c_str());
    EXPECT_TRUE(root.getBoolean("bool1"));
    EXPECT_FALSE(root.getBoolean("bool2"));

    json::ArrayData& array = root.getArray("array");
    Strings skills;
    skills.resize(array.size());
    transform(array.begin(), array.end(), skills.begin(),
              [](const json::Element* skill) { return skill->getString(); });
    EXPECT_STREQ("C++,Java,Python", skills.join(",").c_str());

    const json::Element* object = root.find("object");
    EXPECT_TRUE(object != nullptr);
    EXPECT_EQ(testInteger, object->getNumber("height"));
    EXPECT_DOUBLE_EQ(testDouble2, object->getNumber("weight"));
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

TEST(SPTK_JsonDocument, clear)
{
    json::Document document;
    document.load(testJSON);

    document.clear();
    json::Element& root = document.root();
    EXPECT_TRUE(root.is(Type::OBJECT));
    EXPECT_FALSE(root.find("address"));
    EXPECT_EQ(root.size(), size_t(0));
}

TEST(SPTK_JsonDocument, exportToBuffer)
{
    json::Document document;
    document.load(testJSON);

    Buffer buffer;
    document.exportTo(buffer, false);

    document.load(testJSON);
    verifyDocument(document);
}

TEST(SPTK_JsonDocument, exportToStream)
{
    json::Document document;
    document.load(testJSON);

    stringstream stream;
    document.exportTo(stream, false);

    document.load(stream);
    verifyDocument(document);
}

TEST(SPTK_JsonDocument, copyCtor)
{
    json::Document document;
    document.load(testJSON);

    json::Document document2(document);

    verifyDocument(document);
    verifyDocument(document2);
}

TEST(SPTK_JsonDocument, moveCtor)
{
    json::Document document;
    document.load(testJSON);

    json::Document document2(move(document));

    verifyDocument(document2);
}

TEST(SPTK_JsonDocument, copyAssign)
{
    json::Document document;
    document.load(testJSON);

    json::Document document2;

    document2 = document;

    verifyDocument(document);
    verifyDocument(document2);
}

TEST(SPTK_JsonDocument, moveAssign)
{
    json::Document document;
    document.load(testJSON);

    json::Document document2;

    document2 = move(document);

    verifyDocument(document2);
}

TEST(SPTK_JsonDocument, truncated)
{
    json::Document document;
    String truncatedJSON = testJSON.substr(0, testJSON.length() - 3);
    try {
        document.load(truncatedJSON);
        FAIL() << "Incorrect: MUST fail";
    }
    catch (const Exception& e) {
        SUCCEED() << "Correct: " << e.what();
    }
}

TEST(SPTK_JsonDocument, errors)
{
    json::Document document;
    size_t errorCount = 0;

    String junkTailJSON = String(testJSON) + "=";
    try {
        document.load(junkTailJSON);
        FAIL() << "Incorrect: MUST fail";
    }
    catch (const Exception&) {
        ++errorCount;
    }

    String junkInsideJSON = testJSON;
    junkInsideJSON[0] = '?';
    try {
        document.load(junkInsideJSON);
        FAIL() << "Incorrect: MUST fail";
    }
    catch (const Exception&) {
        ++errorCount;
    }

    try {
        document.load(testJSON);
        const auto* element = document.root().find("nothing");
        if (element != nullptr)
            FAIL() << "Incorrect: MUST return null";
        const auto& root = document.root();
        element = root.find("name");
        element = element->find("nothing");
        if (element != nullptr)
            FAIL() << "Incorrect: MUST fail";
        FAIL() << "Incorrect: MUST fail";
    }
    catch (const Exception&) {
        ++errorCount;
    }

    SUCCEED() << "Detected " << errorCount << " errors";
}

TEST(SPTK_JsonDocument, performance)
{
    constexpr int objectCount = 50000;

    json::Document document;

    auto* arrayElement = document.root().add_array("items");
    for (int i = 0; i < objectCount; ++i) {
        auto& object = *arrayElement->push_object();
        object.set("id", i);
        object.set("name", "Name " + to_string(i));
        object.set("exists", true);

        auto& address = *object.add_object("address");
        address["number"] = i;
        address["street"] = String("Street " + to_string(i));

        auto& list = *address.add_array("list");
        list.push_back(1);
        list.push_back("two");
        list.push_back(3);
    }

    // Verify data
    auto& arrayData = arrayElement->getArray();
    constexpr int someIndex = 100;
    auto& object = arrayData[someIndex];
    EXPECT_FLOAT_EQ(object.getNumber("id"), 100.0);
    EXPECT_STREQ(object.getString("name").c_str(), "Name 100");

    auto& address = object["address"];
    EXPECT_FLOAT_EQ(address.getNumber("number"), 100.0);
    EXPECT_STREQ(address.getString("street").c_str(), "Street 100");
    auto& list = address.getArray("list");
    EXPECT_STREQ(list[1].getString().c_str(), "two");

    Buffer buffer;
    document.exportTo(buffer, true);

    StopWatch stopWatch;
    stopWatch.start();

    json::Document document1;
    document1.load(buffer.c_str());

    stopWatch.stop();

    COUT("Parsed JSON document (" << objectCount << ") objects for " << stopWatch.seconds() << " seconds" << endl)
}

#endif
