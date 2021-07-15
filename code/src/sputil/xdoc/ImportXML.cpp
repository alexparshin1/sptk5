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

#include <cstdlib>
#include <sptk5/Strings.h>
#include <sptk5/xdoc/ImportXML.h>
#include <sptk5/xdoc/Document.h>
#include <sptk5/Printer.h>
#include <cmath>

using namespace std;
using namespace sptk;
using namespace xdoc;

namespace sptk::xdoc {

const RegularExpression ImportXML::parseAttributes {R"(([\w\-_\.:]+)\s*=\s*['"]([^'"]+)['"])", "g"};

void ImportXML::processAttributes(Node& node, const char* ptr)
{
    auto matches = parseAttributes.m(ptr);

    for (auto itor = matches.groups().begin(); itor != matches.groups().end(); itor += 2)
    {
        const auto& attributeName = itor->value;
        auto vtor = itor + 1;
        if (vtor == matches.groups().end())
        {
            break;
        }
        m_encodeBuffer.bytes(0);
        m_doctype.decodeEntities(vtor->value.c_str(), (uint32_t) vtor->value.length(), m_encodeBuffer);
        node.setAttribute(attributeName, m_encodeBuffer.c_str());
    }
}

char* ImportXML::parseEntity(char* start)
{
    static const RegularExpression matchEntity(R"( (?<name>[\w_\-]+)\s+["'](?<value>.*)["'])");
    constexpr int entityMarkerLength = 8;

    start = strstr(start, "<!ENTITY ");
    if (start == nullptr)
    {
        return start;
    }
    start += entityMarkerLength;

    auto* end = strchr(start, '>');
    if (end == nullptr)
    {
        return end;
    }

    *end = 0;

    if (auto matches = matchEntity.m(start);
        matches)
    {
        m_doctype.setEntity(matches["name"].value, matches["value"].value);
    }
    return end + 1;
}

void ImportXML::parseEntities(char* entitiesSection)
{
    auto* start = entitiesSection;
    while (start != nullptr)
    {
        start = parseEntity(start);
    }
}

unsigned char* ImportXML::skipSpaces(unsigned char* start)
{
    while (*start <= ' ')
    {
        ++start;
    }
    return start;
}

void ImportXML::parseXMLDocType(char* docTypeSection)
{
    m_doctype.m_name = "";
    m_doctype.m_public_id = "";
    m_doctype.m_system_id = "";
    m_doctype.m_entities.clear();
    char* start = docTypeSection;
    int index = 0;
    int t = 0;

    extractEntities(docTypeSection);

    char delimiter = ' ';
    while (start != nullptr)
    {
        while (*start == ' ' || *start == delimiter)
        {
            ++start;
        }
        char* end = strchr(start, delimiter);
        if (end != nullptr)
        {
            *end = 0;
        }
        switch (index)
        {
            case 0:
                m_doctype.m_name = start;
                if (end == nullptr)
                {
                    return;
                }
                break;
            case 1:
            case 3:
                if (end == nullptr)
                {
                    break;
                }
                if (strcmp(start, "SYSTEM") == 0)
                {
                    t = 0;
                }
                else if (strcmp(start, "PUBLIC") == 0)
                {
                    t = 1;
                }
                delimiter = '\"';
                break;
            case 2:
            case 4:
                switch (t)
                {
                    case 0:
                        m_doctype.m_system_id = start;
                        break;
                    case 1:
                        m_doctype.m_public_id = start;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
        if (end == nullptr)
        {
            break;
        }
        start = end + 1;
        ++index;
    }
}

void ImportXML::extractEntities(char* docTypeSection)
{
    char* entitiesSection = strchr(docTypeSection, '[');
    if (entitiesSection != nullptr)
    {
        *entitiesSection = 0;
        ++entitiesSection;
        char* end = strchr(entitiesSection, ']');
        if (end != nullptr)
        {
            *end = 0;
            parseEntities(entitiesSection);
        }
    }
}

char* ImportXML::readComment(Node& currentNode, char* nodeName, char* nodeEnd, char* tokenEnd)
{
    nodeEnd = strstr(nodeName + 3, "-->");
    if (nodeEnd == nullptr)
    {
        throw Exception("Invalid end of the comment tag");
    }
    *nodeEnd = 0;
    currentNode.pushNode(nodeName + 3, Node::Type::Comment);
    tokenEnd = nodeEnd + 2;
    return tokenEnd;
}

char* ImportXML::readCDataSection(Node& currentNode, char* nodeName, char* nodeEnd, char* tokenEnd,
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
        currentNode.pushNode("#cdata", Node::Type::CData)
                   .setString(nodeName + cdataTagLength);
    }
    else
    {
        if (currentNode.empty())
        {
            currentNode.type(Node::Type::CData);
            currentNode.setString(nodeName + cdataTagLength);
        }
    }
    tokenEnd = nodeEnd + 2;
    return tokenEnd;
}

char* ImportXML::readXMLDocType(char* tokenEnd)
{
    auto* nodeEnd = strstr(tokenEnd + 1, "]>");

    if (nodeEnd != nullptr)
    { /// ENTITIES
        ++nodeEnd;
        *nodeEnd = 0;
    }
    else
    {
        nodeEnd = strchr(tokenEnd + 1, '>');
        if (nodeEnd == nullptr)
        {
            throw Exception("Invalid CDATA section");
        }
        *nodeEnd = 0;
    }

    parseXMLDocType(tokenEnd + 1);
    tokenEnd = nodeEnd;

    return tokenEnd;
}

char* ImportXML::readExclamationTag(Node& currentNode, char* nodeName, char* tokenEnd, char* nodeEnd, Mode formatting)
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
    else if (strncmp(nodeName, "!DOCTYPE", docTypeTagLength) == 0 && ch != '>')
    {
        tokenEnd = readXMLDocType(tokenEnd);
    }
    return tokenEnd;
}

char* ImportXML::readProcessingInstructions(Node& currentNode, const char* nodeName, char* tokenEnd, char*& nodeEnd,
                                            bool isRootNode)
{
    nodeEnd = strstr(tokenEnd, "?>");
    if (nodeEnd == nullptr)
    {
        throw Exception("Invalid PI section: no closing tag");
    }
    *nodeEnd = 0;
    *tokenEnd = 0;
    Node* pi;
    if (isRootNode)
    {
        pi = &currentNode;
        pi->name(nodeName + 1);
        pi->type(Node::Type::ProcessingInstruction);
    }
    else
    {
        pi = &currentNode.pushNode(nodeName + 1, Node::Type::ProcessingInstruction);
    }

    processAttributes(*pi, tokenEnd + 1);

    ++nodeEnd;
    tokenEnd = nodeEnd;
    return tokenEnd;
}

char* ImportXML::readClosingTag(Node*& currentNode, const char* nodeName, char* tokenEnd, char*& nodeEnd)
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

    currentNode = currentNode->parent();

    nodeEnd = tokenEnd;

    return tokenEnd;
}

char* ImportXML::readOpenningTag(Node*& currentNode, const char* nodeName, char* tokenEnd, char*& nodeEnd)
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
            currentNode = &currentNode->pushNode(nodeName, Node::Type::Null);
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
    Node* anode;
    if (*nodeEnd == '/')
    {
        anode = &currentNode->pushNode(nodeName, Node::Type::Null);
        *nodeEnd = 0;
        ++nodeEnd;
    }
    else
    {
        anode = currentNode = &currentNode->pushNode(nodeName, Node::Type::Null);
        *nodeEnd = 0;
    }
    processAttributes(*anode, tokenStart);
    tokenEnd = nodeEnd;
    return tokenEnd;
}

void ImportXML::detectArray(Node& _node)
{
    if (!_node.is(Node::Type::Object) || _node.size() < 2)
    {
        return;
    }

    // Check if all the child nodes have the same name:
    bool first = true;
    String itemName;
    for (const auto& node: _node)
    {
        if (first)
        {
            first = false;
            itemName = node.name();
        }
        else
        {
            if (itemName != node.name())
            {
                return;
            }
        }
    }

    // All the child nodes have the same name.
    // Convert node to array
    _node.type(Node::Type::Array);
}

void ImportXML::parse(Node& node, const char* _buffer, Mode formatting)
{
    node.clear();
    Node* currentNode = &node;
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
        switch (*nameStart)
        {
            case '!':
                nodeEnd = readExclamationTag(*currentNode, nodeName, nameEnd, nodeEnd, formatting);
                break;

            case '?':
                readProcessingInstructions(*currentNode, nodeName, nameEnd, nodeEnd, false);
                break;

            case '/':
                readClosingTag(currentNode, nodeName, nameEnd, nodeEnd);
                detectArray(*currentNode);
                break;

            default:
                readOpenningTag(currentNode, nodeName, nameEnd, nodeEnd);
                break;
        }

        nodeStart = strchr(nodeEnd + 1, '<');
        if (nodeStart == nullptr)
        {
            if (currentNode == &node)
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

            readText(*currentNode, doctype, nodeStart, textStart, formatting);
        }
    }
}

void ImportXML::readText(Node& currentNode, XMLDocType* doctype, const char* nodeStart, const char* textStart,
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
                auto value = std::stod(decodedText);
                currentNode.setFloat(value);
                currentNode.type(Node::Type::Number);
                nodeType = Node::Type::Number;
            }
            catch (const invalid_argument&)
            {
                nodeType = Node::Type::Text;
            }
            catch (const out_of_range&)
            {
                nodeType = Node::Type::Text;
            }
            currentNode.type(nodeType);
        }

        if (formatting == Mode::KeepFormatting) // || decodedText.find_first_not_of("\n\r\t ") != string::npos)
        {
            currentNode.pushNode("#text", nodeType)
                       .setString(decodedText);
        }
        else
        {
            if (nodeType == Node::Type::Text)
            {
                currentNode.setString(decodedText);
                currentNode.type(nodeType);
            }
        }
    }
}

bool ImportXML::isNumber(const String& str)
{
    static const RegularExpression matchNumber {R"(^[+\-]?(0|[1-9]\d*)(\.\d+)?(e-?\d+)?$)", "i"};

    return matchNumber.matches(str);
}

} // namespace sptk

#if USE_GTEST

static const String testXML("<name position='president'>John</name>"
                            "<age>33</age>"
                            "<temperature>36.6</temperature>"
                            "<timestamp>1519005758000</timestamp>"
                            "<skills><skill>C++</skill><skill>Java</skill><skill>Motorbike</skill></skills>"
                            "<address><married>true</married><employed>false</employed></address>"
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

static void verifyDocument(Document& document)
{
    const Node* nameNode = document.root().findFirst("name");
    EXPECT_STREQ("John", nameNode->asString().c_str());
    EXPECT_STREQ("president", nameNode->getAttribute("position").c_str());

    EXPECT_EQ(33, (int) document.getNumber("age"));
    EXPECT_DOUBLE_EQ(36.6, document.getNumber("temperature"));
    EXPECT_DOUBLE_EQ(1519005758, (int) (document.getNumber("timestamp") / 1000));

    Strings skills;
    for (const auto& node: *document.findFirst("skills"))
    {
        skills.push_back(node.getString());
    }
    EXPECT_STREQ("C++,Java,Motorbike", skills.join(",").c_str());

    const Node* ptr = document.findFirst("address");
    EXPECT_TRUE(ptr != nullptr);

    const Node& address = *ptr;
    EXPECT_STREQ("true", address.getString("married").c_str());
    EXPECT_STREQ("false", address.getString("employed").c_str());

    const Node* dataNode = document.findFirst("data");

    for (const auto& cdataNode: *dataNode)
    {
        EXPECT_TRUE(cdataNode.is(Node::Node::Type::CData));
        EXPECT_STREQ("hello, /\\>", cdataNode.getString().c_str());
    }
}

TEST(SPTK_XDocument, loadXML)
{
    Document document;
    document.load(DataFormat::XML, testXML);
    verifyDocument(document);
}

TEST(SPTK_XDocument, addNodes)
{
    Document document;
    document.load(DataFormat::XML, testXML);

    document.pushNode("name") = String("John");
    document.pushNode("age") = String("33");
    document.pushNode("temperature") = String("33.6");
    document.pushNode("timestamp") = String("1519005758000");

    auto& skills = document.pushNode("skills");
    skills.pushNode("skill") = String("C++");
    skills.pushNode("skill") = String("Java");
    skills.pushNode("skill") = String("Motorbike");

    auto& address = document.pushNode("address");
    address.pushNode("married") = String("true");
    address.pushNode("employed") = String("false");

    verifyDocument(document);
}

TEST(SPTK_XDocument, removeNodes)
{
    Document document;
    document.load(DataFormat::XML, testXML);

    document.findOrCreate("name");
    document.findOrCreate("age");
    document.findOrCreate("skills");
    document.findOrCreate("address");

    document.remove("name");
    document.remove("age");
    document.remove("skills");
    document.remove("address");
    EXPECT_TRUE(document.findFirst("name") == nullptr);
    EXPECT_TRUE(document.findFirst("age") == nullptr);
    EXPECT_TRUE(document.findFirst("temperature") != nullptr);
    EXPECT_TRUE(document.findFirst("skills") == nullptr);
    EXPECT_TRUE(document.findFirst("address") == nullptr);
}

TEST(SPTK_XDocument, saveXml1)
{
    Document document;
    document.load(DataFormat::XML, testREST);

    Buffer buffer;

    document.exportTo(DataFormat::XML, buffer, false);

    EXPECT_STREQ(testREST.c_str(), buffer.c_str());
}

TEST(SPTK_XDocument, saveXml2)
{
    Document document;
    document.load(DataFormat::XML, testXML);

    Buffer buffer;
    document.exportTo(DataFormat::XML, buffer, false);

    document.load(DataFormat::XML, buffer);
    verifyDocument(document);
}

TEST(SPTK_XDocument, parseXML)
{
    Document document;
    document.load(DataFormat::XML, testREST);

    const auto* xmlElement = document.findFirst("xml");
    EXPECT_STREQ(xmlElement->getAttribute("version").c_str(), "1.0");
    EXPECT_STREQ(xmlElement->getAttribute("encoding").c_str(), "UTF-8");

    const auto* bodyElement = document.findFirst("soap:Body", Node::SearchMode::Recursive);
    if (bodyElement == nullptr)
        FAIL() << "Node soap:Body not found";
    EXPECT_EQ(Node::Node::Type::Object, bodyElement->type());
    EXPECT_EQ(1, (int) bodyElement->size());
    EXPECT_STREQ("soap:Body", bodyElement->name().c_str());

    const Node* methodElement = nullptr;
    for (const auto& node: *bodyElement)
    {
        if (node.is(Node::Node::Type::Object))
        {
            methodElement = &node;
            break;
        }
    }
    EXPECT_TRUE(methodElement != nullptr);
    EXPECT_EQ(2, (int) methodElement->size());
    EXPECT_STREQ("ns1:GetRequests", methodElement->name().c_str());
}

TEST(SPTK_XDocument, brokenXML)
{
    Document document;

    try
    {
        const String brokenXML1("<xml></html>");
        document.load(DataFormat::XML, brokenXML1);
        FAIL() << "Must throw exception";
    }
    catch (const Exception& e)
    {
        SUCCEED() << "Correct exception: " << e.what();
    }

    try
    {
        const String brokenXML1("<xml><html></xml></html>");
        document.load(DataFormat::XML, brokenXML1);
        FAIL() << "Must throw exception";
    }
    catch (const Exception& e)
    {
        SUCCEED() << "Correct exception: " << e.what();
    }

    try
    {
        const String brokenXML1("<xml</html>");
        document.load(DataFormat::XML, brokenXML1);
        FAIL() << "Must throw exception";
    }
    catch (const Exception& e)
    {
        SUCCEED() << "Correct exception: " << e.what();
    }
}

TEST(SPTK_XDocument, unicodeAndSpacesXML)
{
    Document document;

    try
    {
        const String unicodeXML(R"(<?xml encoding="UTF-8" version="1.0"?><p> “Add” </p><span> </span>)");
        document.load(DataFormat::XML, unicodeXML, true);
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
    document.load(DataFormat::XML, input);

    Buffer output;
    document.exportTo(DataFormat::JSON, output, true);

    COUT(output.c_str() << endl)
}

TEST(SPTK_XDocument, loadFormattedXML)
{
    Buffer input;
    input.loadFromFile("data/content2.xml");

    Document document;
    document.load(DataFormat::XML, input, true);

    Buffer output;
    document.exportTo(DataFormat::XML, output, false);
    output.saveToFile("data/content2_exp.xml");
}

#endif
