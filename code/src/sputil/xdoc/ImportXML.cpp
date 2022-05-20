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

#include <cmath>
#include <cstdlib>
#include <sptk5/Printer.h>
#include <sptk5/Strings.h>
#include <sptk5/xdoc/Document.h>
#include <sptk5/xdoc/ImportXML.h>

using namespace std;
using namespace sptk;
using namespace xdoc;

namespace sptk::xdoc {

const RegularExpression ImportXML::parseAttributes {R"(([\w\-_\.:]+)(\s*=\s*))", "g"};

void ImportXML::processAttributes(Node& node, const char* ptr)
{
    auto matches = parseAttributes.m(ptr);

    for (auto itor = matches.groups().begin(); itor != matches.groups().end(); itor += 2)
    {
        auto vtor = itor + 1;
        if (vtor == matches.groups().end())
        {
            throw Exception("Invalid attribute format for " + node.name() + " tag");
        }

        const auto& attributeName = itor->value;
        const char* attributeStart = ptr + vtor->end;
        const char* attributeEnd = nullptr;

        auto expectedChar = *attributeStart;

        switch (expectedChar)
        {
            case '\'':
            case '"':
                ++attributeStart;
                attributeEnd = strchr(attributeStart, expectedChar);
                break;
            default:
                expectedChar = ' ';
                attributeEnd = strpbrk(attributeStart, " ");
                break;
        }

        size_t attributeLength;
        if (attributeEnd == nullptr)
        {
            attributeLength = strlen(attributeStart);
        }
        else
        {
            if (*attributeEnd != expectedChar)
            {
                throw Exception("Invalid attribute format for " + node.name() + " tag");
            }
            attributeLength = attributeEnd - attributeStart;
        }

        m_encodeBuffer.bytes(0);
        m_doctype.decodeEntities(attributeStart, attributeLength, m_encodeBuffer);
        node.attributes().set(attributeName, m_encodeBuffer.c_str());
    }
}

char* ImportXML::readComment(const SNode& currentNode, char* nodeName, char* nodeEnd, char* tokenEnd)
{
    nodeEnd = strstr(nodeName + 3, "-->");
    if (nodeEnd == nullptr)
    {
        throw Exception("Invalid end of the comment tag");
    }
    *nodeEnd = 0;
    currentNode->pushNode(nodeName + 3, Node::Type::Comment);
    tokenEnd = nodeEnd + 2;
    return tokenEnd;
}

char* ImportXML::readCDataSection(const SNode& currentNode, char* nodeName, char* nodeEnd, char* tokenEnd,
                                  Mode formatting)
{
    constexpr int cdataTagLength = 8;
    nodeEnd = strstr(nodeName + 1, "]]>");
    if (nodeEnd == nullptr)
    {
        throw Exception("Invalid CDATA section");
    }
    *nodeEnd = 0;
    if (formatting == Mode::KeepFormatting)
    {
        currentNode->pushValue("#cdata", nodeName + cdataTagLength, Node::Type::CData);
    }
    else
    {
        if (currentNode->nodes().empty())
        {
            currentNode->type(Node::Type::CData);
            currentNode->set(nodeName + cdataTagLength);
        }
    }
    tokenEnd = nodeEnd + 2;
    return tokenEnd;
}

char* ImportXML::readXMLDocType(char* tokenEnd)
{
    tokenEnd = strstr(tokenEnd + 1, "]>");
    return tokenEnd;
}

char* ImportXML::readExclamationTag(const SNode& currentNode, char* nodeName, char* tokenEnd, char* nodeEnd,
                                    Mode formatting)
{
    constexpr int cdataTagLength = 8;
    constexpr int docTypeTagLength = 8;
    char ch = *tokenEnd;
    *tokenEnd = 0;
    if (strncmp(nodeName, "!--", 3) == 0)
    {
        /// Comment
        *tokenEnd = ch; // ' ' or '>' could be within a comment
        tokenEnd = readComment(currentNode, nodeName, nodeEnd, tokenEnd);
    }
    else if (strncmp(nodeName, "![CDATA[", cdataTagLength) == 0)
    {
        /// CDATA section
        *tokenEnd = ch;
        tokenEnd = readCDataSection(currentNode, nodeName, nodeEnd, tokenEnd, formatting);
    }
    else if (strncmp(nodeName, "!DOCTYPE", docTypeTagLength) == 0 && ch != '>' && *tokenEnd)
    {
        tokenEnd = readXMLDocType(tokenEnd);
    }
    return tokenEnd;
}

char* ImportXML::readProcessingInstructions(const SNode& currentNode, const char* nodeName, char* tokenEnd,
                                            char*& nodeEnd, bool isRootNode)
{
    nodeEnd = strstr(tokenEnd, "?>");
    if (nodeEnd == nullptr)
    {
        throw Exception("Invalid PI section: no closing tag");
    }
    *nodeEnd = 0;
    *tokenEnd = 0;
    SNode pi;
    if (isRootNode)
    {
        pi = currentNode;
        pi->name(nodeName + 1);
        pi->type(Node::Type::ProcessingInstruction);
    }
    else
    {
        pi = currentNode->pushNode(nodeName + 1, Node::Type::ProcessingInstruction);
    }

    processAttributes(*pi, tokenEnd + 1);

    ++nodeEnd;
    tokenEnd = nodeEnd;
    return tokenEnd;
}

char* ImportXML::readClosingTag(const SNode& currentNode, const char* nodeName, char* tokenEnd, char*& nodeEnd)
{
    char ch = *tokenEnd;
    *tokenEnd = 0;
    if (ch != '>')
    {
        throw Exception("Invalid tag (spaces before closing '>')");
    }
    ++nodeName;
    if (currentNode->name() != nodeName)
    {
        throw Exception(
            "Closing tag <" + string(nodeName) + "> doesn't match opening <" + currentNode->name() + ">");
    }

    nodeEnd = tokenEnd;

    return tokenEnd;
}

char* ImportXML::readOpenningTag(SNode& currentNode, const char* nodeName, char* tokenEnd, char*& nodeEnd)
{
    char ch = *tokenEnd;
    *tokenEnd = 0;
    if (ch == '>' || ch == '/')
    {
        if (ch == '/')
        {
            currentNode->pushNode(nodeName, Node::Type::Null);
            nodeEnd = tokenEnd + 1;
        }
        else
        {
            currentNode = currentNode->pushNode(nodeName, Node::Type::Null);
            nodeEnd = tokenEnd;
        }
        return tokenEnd;
    }

    auto* tokenStart = tokenEnd + 1;
    nodeEnd = strchr(tokenStart, '>');
    if (nodeEnd == nullptr)
    {
        throw Exception("Invalid tag (started, not closed)");
    }

    if (auto len = nodeEnd - tokenStart;
        tokenStart[len - 1] == '/')
    {
        nodeEnd = tokenStart + len - 1;
    }

    /// Attributes
    SNode anode;
    if (*nodeEnd == '/')
    {
        anode = currentNode->pushNode(nodeName, Node::Type::Null);
        *nodeEnd = 0;
        ++nodeEnd;
    }
    else
    {
        anode = currentNode = currentNode->pushNode(nodeName, Node::Type::Null);
        *nodeEnd = 0;
    }
    processAttributes(*anode, tokenStart);
    tokenEnd = nodeEnd;
    return tokenEnd;
}

SNode ImportXML::detectArray(const SNode& _node)
{
    if (_node->type() != Node::Type::Object || _node->nodes().size() < 2)
    {
        return _node;
    }

    // Check if all the child nodes have the same name:
    bool first = true;
    String itemName;
    for (const auto& node: _node->nodes())
    {
        if (first)
        {
            first = false;
            itemName = node->name();
        }
        else
        {
            if (itemName != node->name())
            {
                return _node;
            }
        }
    }
    _node->type(Node::Type::Array);

    return _node;
}

void ImportXML::parse(const SNode& node, const char* _buffer, Mode formatting)
{
    node->clear();
    SNode currentNode = node;
    XMLDocType* doctype = &docType();
    Buffer buffer(_buffer);

    for (char* nodeStart = strchr((char*) buffer.data(), '<'); nodeStart != nullptr;)
    {
        auto* nameStart = nodeStart + 1;
        char* nameEnd = strpbrk(nameStart + 1, "?/\r\n <>");
        if (nameEnd == nullptr || *nameEnd == '<')
        {
            throw Exception("Tag started but not closed");
        }

        char* nodeName = nameStart;
        char* nodeEnd = nameStart;
        bool autoClosed = false;
        switch (*nameStart)
        {
            case '!':
                nodeEnd = readExclamationTag(currentNode, nodeName, nameEnd, nodeEnd, formatting);
                break;

            case '?':
                readProcessingInstructions(currentNode, nodeName, nameEnd, nodeEnd, false);
                break;

            case '/':
                readClosingTag(currentNode, nodeName, nameEnd, nodeEnd);
                detectArray(currentNode);
                currentNode = currentNode->parent();
                break;

            default:
                readOpenningTag(currentNode, nodeName, nameEnd, nodeEnd);
                // For HTML, autoclose 'meta' tags
                if (strcmp(nodeName, "meta") == 0 && currentNode->parent()->name().in({"html", "head"}))
                {
                    autoClosed = true;
                    currentNode = currentNode->parent();
                }
                break;
        }

        nodeStart = strchr(nodeEnd + 1, '<');
        if (nodeStart == nullptr)
        {
            if (currentNode == node)
            {
                continue;
            }
            // exit the loop
            return;
        }

        const auto* textStart = nodeEnd + 1;
        if (*textStart != '<')
        {
            if (formatting == Mode::Compact)
            {
                auto skipSpaces = strspn(textStart, "\n\r\t ");
                textStart += skipSpaces;
            }

            if (!autoClosed)
                readText(currentNode, doctype, nodeStart, textStart, formatting);
        }
    }
}

void ImportXML::readText(const SNode& currentNode, XMLDocType* doctype, const char* nodeStart, const char* textStart,
                         Mode formatting)
{
    const auto* textTrail = nodeStart;
    if (textStart != textTrail)
    {
        Buffer& decoded = m_decodeBuffer;
        doctype->decodeEntities(textStart, uint32_t(textTrail - textStart), decoded);
        String decodedText(decoded.c_str(), decoded.length());

        Node::Type nodeType = Node::Type::Text;
        if (formatting != Mode::KeepFormatting)
        {
            decodedText = decodedText.trim();
        }

        if (decodedText[0] >= '+' && decodedText[0] <= '9')
        {
            try
            {
                if (isInteger(decodedText))
                {
#ifdef _WIN32
                    auto value = std::stoll(decodedText);
#else
                    auto value = std::stol(decodedText);
#endif
                    currentNode->set(value);
                    currentNode->type(Node::Type::Number);
                    nodeType = Node::Type::Number;
                }
                else if (isFloat(decodedText))
                {
                    auto value = std::stod(decodedText);
                    currentNode->set(value);
                    currentNode->type(Node::Type::Number);
                    nodeType = Node::Type::Number;
                }
            }
            catch (const invalid_argument&)
            {
                nodeType = Node::Type::Text;
            }
            catch (const out_of_range&)
            {
                nodeType = Node::Type::Text;
            }
            currentNode->type(nodeType);
        }

        if (nodeType != Node::Type::Number && formatting == Mode::KeepFormatting) // || decodedText.find_first_not_of("\n\r\t ") != string::npos)
        {
            currentNode->pushNode("#text", nodeType)
                ->set(decodedText);
        }
        else
        {
            if (nodeType == Node::Type::Text)
            {
                currentNode->set(decodedText);
                currentNode->type(nodeType);
            }
        }
    }
}

#ifdef USE_GTEST

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

    EXPECT_EQ(33, (int) document.root()->getNumber("age"));
    EXPECT_DOUBLE_EQ(36.6, document.root()->getNumber("temperature"));
    EXPECT_DOUBLE_EQ(1519005758, (int) (document.root()->getNumber("timestamp") / 1000));

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
    Document document;
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
    EXPECT_EQ(Node::Node::Type::Object, bodyElement->type());
    EXPECT_EQ(1, (int) bodyElement->nodes().size());
    EXPECT_STREQ("soap:Body", bodyElement->name().c_str());

    SNode methodElement = nullptr;
    for (const auto& node: bodyElement->nodes())
    {
        if (node->type() == Node::Node::Type::Object)
        {
            methodElement = node;
            break;
        }
    }
    EXPECT_TRUE(methodElement != nullptr);
    EXPECT_EQ(2, (int) methodElement->nodes().size());
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
    Document document;

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
    Buffer input(testXML);
    Document document;
    document.load(input);

    Buffer output;
    document.exportTo(DataFormat::JSON, output, true);

    COUT(output.c_str() << endl)
}

TEST(SPTK_XDocument, loadFormattedXML)
{
    Buffer input;
    input.loadFromFile("data/content2.xml");

    Document document;
    document.load(input, true);

    Buffer output;
    document.exportTo(DataFormat::XML, output, false);
    output.saveToFile("data/content2_exp.xml");
}

TEST(SPTK_XDocument, getText)
{
    Document document;
    document.load(testOO, true);
    auto text = document.root()->getText();
    EXPECT_STREQ(text.c_str(), "Fig. PE 6.1.2 ImportAudio File");
}

TEST(SPTK_XDocument, unquotedXmlAttributes)
{
    const Buffer unquotedAttributesXml(String("<data><name value=Alex type=first /><last_name value=Doe type=last/></data>"));
    Document document;
    document.load(unquotedAttributesXml, true);

    auto firstName = document.root()->findFirst("name");
    EXPECT_TRUE(firstName != nullptr);
    EXPECT_STREQ("Alex", firstName->attributes().get("value").c_str());

    auto lastName = document.root()->findFirst("last_name");
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

    auto head = document.root()->findFirst("head");
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

#endif

} // namespace sptk::xdoc
