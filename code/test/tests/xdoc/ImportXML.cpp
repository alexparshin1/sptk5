/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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

#include <cmath>
#include <gtest/gtest.h>
#include <sptk5/Printer.h>
#include <sptk5/Strings.h>
#include <sptk5/xdoc/Document.h>

using namespace std;
using namespace sptk;
using namespace xdoc;

static const String testXML("<name position='president'>John</name>"
                            "<age>33</age>"
                            "<temperature>36.6</temperature>"
                            "<timestamp>1519005758000</timestamp>"
                            "<skills><skill>C++</skill><skill>Java</skill><skill>Motorbike</skill></skills>"
                            "<address><married>true</married><employed>false</employed></address>"
                            "<text>Once upon a time, in a <bold>far away kingdom</bold></text>"
                            "<data><![CDATA[hello, /\\>]]></data>");

static const String testREST(
    R"(<?xml version="1.0" encoding="UTF-8"?>)"
    R"(<soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">)"
    R"(<soap:Body>)"
    R"(<ns1:GetRequests>)"
    R"(<vendor_id>1</vendor_id><address state="VIC"/>)"
    R"(</ns1:GetRequests>)"
    R"(</soap:Body>)"
    R"(</soap:Envelope>)");

static const String testOO(
    R"(<text:p text:style-name="Figure_20_Header"><text:bookmark text:name="F_6_1_2"/>Fig.)"
    R"( PE <text:span text:style-name="T94">6</text:span>.<text:span text:style-name="T94">1.2)"
    R"(</text:span> Import)"
    R"(<text:span text:style-name="T94">Audio File</text:span>)"
    R"(</text:p>)");

static void verifyDocument(Document& document)
{
    const auto nameNode = document.root()->findFirst("name");
    EXPECT_STREQ("John", nameNode->getText().c_str());
    EXPECT_STREQ("president", nameNode->attributes().get("position").c_str());

    EXPECT_EQ(33, static_cast<int>(document.root()->getNumber("age")));
    EXPECT_DOUBLE_EQ(36.6, document.root()->getNumber("temperature"));
    EXPECT_DOUBLE_EQ(1519005758, static_cast<int>(document.root()->getNumber("timestamp") / 1000));

    const auto textNode = document.findFirst("text");
    EXPECT_STREQ("Once upon a time, in a far away kingdom", textNode->getText().c_str());

    Strings skills;
    for (const auto& node: document.root()->nodes("skills"))
    {
        skills.push_back(node->getText());
    }
    EXPECT_STREQ("C++,Java,Motorbike", skills.join(",").c_str());

    const auto ptr = document.root()->findFirst("address");
    EXPECT_TRUE(ptr != nullptr);

    const Node& address = *ptr;
    EXPECT_STREQ("true", address.getText("married").c_str());
    EXPECT_STREQ("false", address.getText("employed").c_str());

    const auto dataNode = document.root()->findFirst("data");

    for (const auto& cdataNode: dataNode->nodes())
    {
        EXPECT_TRUE(cdataNode->type() == Node::Node::Type::CData);
        EXPECT_STREQ("hello, /\\>", cdataNode->getString().c_str());
    }
}

TEST(SPTK_XDocument, loadXML)
{
    Document document;
    document.load(testXML, true);
    verifyDocument(document);
}

TEST(SPTK_XDocument, addNodes)
{
    Document document;
    document.load(testXML, true);

    *document.root()->pushNode("name") = String("John");
    *document.root()->pushNode("age") = String("33");
    *document.root()->pushNode("temperature") = String("33.6");
    *document.root()->pushNode("timestamp") = String("1519005758000");

    auto& skills = *document.root()->pushNode("skills");
    *skills.pushNode("skill") = String("C++");
    *skills.pushNode("skill") = String("Java");
    *skills.pushNode("skill") = String("Motorbike");

    auto& address = *document.root()->pushNode("address");
    *address.pushNode("married") = String("true");
    *address.pushNode("employed") = String("false");

    verifyDocument(document);
}

TEST(SPTK_XDocument, removeNodes)
{
    Document document;
    document.load(testXML);

    document.root()->findOrCreate("name");
    document.root()->findOrCreate("age");
    document.root()->findOrCreate("skills");
    document.root()->findOrCreate("address");

    document.root()->remove("name");
    document.root()->remove("age");
    document.root()->remove("skills");
    document.root()->remove("address");
    EXPECT_TRUE(document.root()->findFirst("name") == nullptr);
    EXPECT_TRUE(document.root()->findFirst("age") == nullptr);
    EXPECT_TRUE(document.root()->findFirst("temperature") != nullptr);
    EXPECT_TRUE(document.root()->findFirst("skills") == nullptr);
    EXPECT_TRUE(document.root()->findFirst("address") == nullptr);
}

TEST(SPTK_XDocument, saveXml1)
{
    const Document document;
    document.load(testREST);

    Buffer buffer;

    document.exportTo(DataFormat::XML, buffer, false);

    EXPECT_STREQ(testREST.c_str(), buffer.c_str());
}

TEST(SPTK_XDocument, saveXml2)
{
    // Import while keeping formatting
    Document document;
    document.load(testXML, true);

    // Export to XML without changing formatting
    Buffer buffer;
    document.exportTo(DataFormat::XML, buffer, false);

    // Import while keeping formatting
    document.load(buffer, true);

    // Check that resulting document is still Ok
    verifyDocument(document);
}

TEST(SPTK_XDocument, parseXML)
{
    Document document;
    document.load(testREST);

    const auto xmlElement = document.root()->findFirst("xml");
    EXPECT_STREQ(xmlElement->attributes().get("version").c_str(), "1.0");
    EXPECT_STREQ(xmlElement->attributes().get("encoding").c_str(), "UTF-8");

    const auto bodyElement = document.root()->findFirst("soap:Body", SearchMode::Recursive);
    if (bodyElement == nullptr)
        FAIL() << "Node soap:Body not found";
    EXPECT_TRUE(Node::Type::Object == bodyElement->type());
    EXPECT_EQ(1, static_cast<int>(bodyElement->nodes().size()));
    EXPECT_STREQ("soap:Body", bodyElement->name().c_str());

    const auto itor = ranges::find_if(bodyElement->nodes(), [](const SNode& node)
                                      {
                                          return node->type() == Node::Type::Object;
                                      });

    const SNode methodElement = itor != bodyElement->nodes().end() ? *itor : nullptr;
    EXPECT_TRUE(methodElement != nullptr);
    EXPECT_EQ(2, static_cast<int>(methodElement->nodes().size()));
    EXPECT_STREQ("ns1:GetRequests", methodElement->name().c_str());
}

TEST(SPTK_XDocument, brokenXML)
{
    Document document;

    String brokenXML1("<xml></html>");
    EXPECT_THROW(document.load(brokenXML1), Exception);

    brokenXML1 = "<xml><html></xml></html>";
    EXPECT_THROW(document.load(brokenXML1), Exception);

    brokenXML1 = "<xml</html>";
    EXPECT_THROW(document.load(brokenXML1), Exception);
}

TEST(SPTK_XDocument, unicodeAndSpacesXML)
{
    const Document document;

    try
    {
        const String unicodeXML(R"(<?xml encoding="UTF-8" version="1.0"?><p> 世界您好 </p><span> </span>)");
        document.load(unicodeXML, true);
        Buffer buffer;
        document.exportTo(DataFormat::XML, buffer, false);
        EXPECT_STREQ(unicodeXML.c_str(), buffer.c_str());
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

TEST(SPTK_XDocument, exportToJSON)
{
    const Buffer   input(testXML);
    const Document document;
    document.load(input);

    Buffer output;
    document.exportTo(DataFormat::JSON, output, true);

    COUT(output.c_str());
}

TEST(SPTK_XDocument, loadFormattedXML)
{
    if (!filesystem::exists("data/content2.xml"))
    {
        GTEST_SKIP();
    }

    Buffer input;
    input.loadFromFile("data/content2.xml");

    const Document document;
    document.load(input, true);

    Buffer output;
    document.exportTo(DataFormat::XML, output, false);
    output.saveToFile("data/content2_exp.xml");
}

TEST(SPTK_XDocument, getText)
{
    Document document;
    document.load(testOO, true);
    const auto text = document.root()->getText();
    EXPECT_STREQ(text.c_str(), "Fig. PE 6.1.2 ImportAudio File");
}

TEST(SPTK_XDocument, unquotedXmlAttributes)
{
    const Buffer unquotedAttributesXml(String("<data><name value=Alex type=first /><last_name value=Doe type=last/></data>"));
    Document     document;
    document.load(unquotedAttributesXml, true);

    const auto firstName = document.root()->findFirst("name");
    EXPECT_TRUE(firstName != nullptr);
    EXPECT_STREQ("Alex", firstName->attributes().get("value").c_str());

    const auto lastName = document.root()->findFirst("last_name");
    EXPECT_TRUE(lastName != nullptr);
    EXPECT_STREQ("Doe", lastName->attributes().get("value").c_str());
}

TEST(SPTK_XDocument, htmlAutoCloseTags)
{
    const Buffer htmlAutoCloseTagsHtml(String(
        R"(<head>
        <meta charset="UTF-8">
        <meta name="description" content="Free Web tutorials">
        </head>)"));

    Document document;
    document.load(htmlAutoCloseTagsHtml, true);

    const auto head = document.root()->findFirst("head");
    for (const auto& meta: head->nodes())
    {
        if (meta->name() == "meta")
        {
            if (meta->attributes().have("charset"))
            {
                EXPECT_STREQ("UTF-8", meta->attributes().get("charset").c_str());
            }
            else
            {
                EXPECT_STREQ("description", meta->attributes().get("name").c_str());
                EXPECT_STREQ("Free Web tutorials", meta->attributes().get("content").c_str());
            }
        }
    }
}
