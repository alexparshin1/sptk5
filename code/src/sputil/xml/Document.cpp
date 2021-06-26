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
#include <sptk5/json/JsonDocument.h>
#include <sptk5/Printer.h>

using namespace std;
using namespace sptk;

namespace sptk::xml {

const RegularExpression Document::parseAttributes {R"(([\w\-_\.:]+)\s*=\s*['"]([^'"]+)['"])", "g"};

Document::Document()
    : Element(*this)
{
}

Document::Document(const String& xml)
    : Element(*this)
{
    Document::load(xml);
}

Document::Document(const char* aname, const char* public_id, const char* system_id)
    : Element(*this),
      m_doctype(aname, public_id, system_id)
{
}

Node* Document::rootNode()
{
    for (auto* nd: *this)
    {
        if (nd->type() == Type::DOM_ELEMENT)
        {
            return nd;
        }
    }
    return nullptr;
}

Node* Document::createElement(const char* tagname)
{
    Node* node = new Element(this, tagname);
    return node;
}

void Document::processAttributes(Node* node, const char* ptr)
{
    auto matches = parseAttributes.m(ptr);

    for (auto itor = matches.groups().begin(); itor != matches.groups().end(); ++itor)
    {
        const auto& attributeName = itor->value;
        ++itor;
        if (itor == matches.groups().end())
        {
            break;
        }
        m_encodeBuffer.bytes(0);
        m_doctype.decodeEntities(itor->value.c_str(), (uint32_t) itor->value.length(), m_encodeBuffer);
        node->setAttribute(attributeName, m_encodeBuffer.c_str());
    }
}

char* Document::parseEntity(char* start)
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
        m_doctype.m_entities.setEntity(matches["name"].value, matches["value"].value);
    }
    return end + 1;
}

void Document::parseEntities(char* entitiesSection)
{
    auto* start = entitiesSection;
    while (start != nullptr)
    {
        start = parseEntity(start);
    }
}

unsigned char* Document::skipSpaces(unsigned char* start)
{
    while (*start <= ' ')
    {
        ++start;
    }
    return start;
}

void Document::parseDocType(char* docTypeSection)
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

void Document::extractEntities(char* docTypeSection)
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

char* Document::readComment(Node* currentNode, char* nodeName, char* nodeEnd, char* tokenEnd)
{
    nodeEnd = strstr(nodeName + 3, "-->");
    if (nodeEnd == nullptr)
    {
        throw Exception("Invalid end of the comment tag");
    }
    *nodeEnd = 0;
    new Comment(currentNode, nodeName + 3);
    tokenEnd = nodeEnd + 2;
    return tokenEnd;
}

char* Document::readCDataSection(Node* currentNode, char* nodeName, char* nodeEnd, char* tokenEnd)
{
    constexpr int cdataTagLength = 8;
    nodeEnd = strstr(nodeName + 1, "]]>");
    if (nodeEnd == nullptr)
    {
        throw Exception("Invalid CDATA section");
    }
    *nodeEnd = 0;
    new CDataSection(currentNode, nodeName + cdataTagLength);
    tokenEnd = nodeEnd + 2;
    return tokenEnd;
}

char* Document::readDocType(char* tokenEnd)
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

    parseDocType(tokenEnd + 1);
    tokenEnd = nodeEnd;

    return tokenEnd;
}

char* Document::readExclamationTag(char* nodeName, char* tokenEnd, char* nodeEnd, Node* currentNode)
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
        tokenEnd = readCDataSection(currentNode, nodeName, nodeEnd, tokenEnd);
    }
    else if (strncmp(nodeName, "!DOCTYPE", docTypeTagLength) == 0 && ch != '>')
    {
        tokenEnd = readDocType(tokenEnd);
    }
    return tokenEnd;
}

char* Document::readProcessingInstructions(const char* nodeName, char* tokenEnd, char*& nodeEnd, Node* currentNode)
{
    nodeEnd = strstr(tokenEnd, "?>");
    if (nodeEnd == nullptr)
    {
        throw Exception("Invalid PI section: no closing tag");
    }
    *nodeEnd = 0;
    *tokenEnd = 0;
    auto* pi = currentNode->add<PI>(nodeName + 1);
    processAttributes(pi, tokenEnd + 1);

    ++nodeEnd;
    tokenEnd = nodeEnd;
    return tokenEnd;
}

char* Document::readClosingTag(const char* nodeName, char* tokenEnd, char*& nodeEnd, Node*& currentNode)
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
    if (currentNode == nullptr)
    {
        throw Exception(
            "Closing tag <" + string(nodeName) + "> doesn't have corresponding opening tag");
    }
    nodeEnd = tokenEnd;
    return tokenEnd;
}

char* Document::readOpenningTag(const char* nodeName, char* tokenEnd, char*& nodeEnd, Node*& currentNode)
{
    char ch = *tokenEnd;
    *tokenEnd = 0;
    if (ch == '>' || ch == '/')
    {
        if (ch == '/')
        {
            new Element(currentNode, nodeName);
            nodeEnd = tokenEnd + 1;
        }
        else
        {
            currentNode = new Element(currentNode, nodeName);
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
        anode = currentNode->add<Element>(nodeName);
        *nodeEnd = 0;
        ++nodeEnd;
    }
    else
    {
        anode = currentNode = currentNode->add<Element>(nodeName);
        *nodeEnd = 0;
    }
    processAttributes(anode, tokenStart);
    tokenEnd = nodeEnd;
    return tokenEnd;
}

void Document::load(const char* _buffer, bool keepSpaces)
{
    clear();
    Node* currentNode = this;
    DocType* doctype = &docType();
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
                nodeEnd = readExclamationTag(nodeName, nameEnd, nodeEnd, currentNode);
                break;

            case '?':
                readProcessingInstructions(nodeName, nameEnd, nodeEnd, currentNode);
                break;

            case '/':
                readClosingTag(nodeName, nameEnd, nodeEnd, currentNode);
                break;

            default:
                readOpenningTag(nodeName, nameEnd, nodeEnd, currentNode);
                break;
        }

        nodeStart = strchr(nodeEnd + 1, '<');
        if (nodeStart == nullptr)
        {
            if (currentNode == this)
            {
                continue;
            } // exit the loop
            throw Exception("Tag started but not closed");
        }

        const auto* textStart = nodeEnd + 1;
        if (*textStart != '<')
        {
            readText(keepSpaces, currentNode, doctype, nodeStart, textStart);
        }
    }
}

void Document::readText(bool keepSpaces, Node* currentNode, DocType* doctype, const char* nodeStart,
                        const char* textStart)
{
    const auto* textTrail = nodeStart;
    if (textStart != textTrail)
    { // && nodeStart[1] == '/)') {
        Buffer& decoded = m_decodeBuffer;
        doctype->decodeEntities(textStart, uint32_t(textTrail - textStart), decoded);
        String decodedText(decoded.c_str(), decoded.length());
        if (keepSpaces || decodedText.find_first_not_of("\n\r\t ") != string::npos)
        {
            new Text(currentNode, decodedText.c_str());
        }
    }
}

void Document::save(Buffer& buffer, int indent) const
{
    const Node* xml_pi = nullptr;

    buffer.reset();

    // Write XML PI
    bool hasXmlPI = false;
    for (const auto* node: *this)
    {
        if (node->type() == Type::DOM_PI && lowerCase(node->name()) == "xml")
        {
            xml_pi = node;
            xml_pi->save(buffer, 0);
            hasXmlPI = true;
            break;
        }
    }

    if (!hasXmlPI)
    {
        buffer.append("<?xml version=\"1.0\" encoding=\"UTF-8\"?>");
        if (indent)
        {
            buffer.append('\n');
        }
    }

    if (!docType().name().empty())
    {
        buffer.append("<!DOCTYPE " + docType().name());
        if (!docType().systemID().empty())
        {
            buffer.append(" SYSTEM \"" + docType().systemID() + "\"");
        }

        if (!docType().publicID().empty())
        {
            buffer.append(" PUBLIC \"" + docType().publicID() + "\"");
        }

        if (!docType().entities().empty())
        {
            buffer.append(" [\n", 3);
            const Entities& entities = docType().entities();
            for (const auto&[name, value]: entities)
            {
                buffer.append("<!ENTITY " + name + " \"" + value + "\">\n");
            }
            buffer.append("]", 1);
        }
        buffer.append('>');
        if (indent)
        { buffer.append('\n'); }
    }

    // call save() method of the first (and hopefully only) node in xml document
    for (const auto* node: *this)
    {
        if (node == xml_pi)
        {
            continue;
        }
        node->save(buffer, indent);
    }
    buffer[buffer.length()] = 0;
}

String Document::name() const
{
    static const String nodeNameString("#document");
    return nodeNameString;
}

void Document::exportTo(json::Element& json) const
{
    for (const auto* node: *this)
    {
        if (node->isPI())
        {
            continue;
        }
        node->exportTo(json);
        json.optimizeArrays("item");
        break;
    }
}

bool Document::isNumber(const String& str)
{
    static const RegularExpression matchNumber {R"(^[+\-]?(0|[1-9]\d*)(\.\d+)?(e-?\d+)?$)", "i"};

    return matchNumber.matches(str);
}

} // namespace sptk

#if USE_GTEST

static const String testXML(
    "<name position='president'>John</name><age>33</age><temperature>36.6</temperature><timestamp>1519005758000</timestamp>"
    "<skills><skill>C++</skill><skill>Java</skill><skill>Motorbike</skill></skills>"
    "<address><married>true</married><employed>false</employed></address><data><![CDATA[hello, /\\>]]></data>");

static const String testREST(
    R"(<?xml version="1.0" encoding="UTF-8"?>)"
    R"(<soap:Envelope xmlns:soap="http://schemas.xmlsoap.org/soap/envelope/">)"
    R"(<soap:Body>)"
    R"(<ns1:GetRequests>)"
    R"(<vendor_id>1</vendor_id><address state="VIC"/>)"
    R"(</ns1:GetRequests>)"
    R"(</soap:Body>)"
    R"(</soap:Envelope>)");

void verifyDocument(xml::Document& document)
{
    const xml::Node* nameNode = document.findOrCreate("name");
    EXPECT_STREQ("John", nameNode->text().c_str());
    EXPECT_STREQ("president", nameNode->getAttribute("position").asString().c_str());

    EXPECT_EQ(33, string2int(document.findOrCreate("age")->text()));
    EXPECT_DOUBLE_EQ(36.6, string2double(document.findOrCreate("temperature")->text()));
    EXPECT_DOUBLE_EQ(1519005758, int(string2int64(document.findOrCreate("timestamp")->text())/1000));

    Strings skills;
    for (const auto* node: *document.findOrCreate("skills"))
    {
        skills.push_back(node->text());
    }
    EXPECT_STREQ("C++,Java,Motorbike", skills.join(",").c_str());

    xml::Node* ptr = document.findFirst("address");
    EXPECT_TRUE(ptr != nullptr);
    auto* ptr2 = dynamic_cast<xml::Element*>(ptr);

    xml::Element& address = *ptr2;
    EXPECT_STREQ("true", address.findOrCreate("married")->text().c_str());
    EXPECT_STREQ("false", address.findOrCreate("employed")->text().c_str());

    const xml::Node* dataNode = document.findOrCreate("data");
    const xml::Node* cdataNode = nullptr;

    for (const auto* node: *dataNode)
    {
        cdataNode = node;
        EXPECT_TRUE(cdataNode->isCDataSection());
        EXPECT_STREQ("hello, /\\>", cdataNode->value().c_str());
    }
    EXPECT_TRUE(cdataNode != nullptr);
}

TEST(SPTK_XmlDocument, load)
{
    xml::Document document;
    document.load(testXML);
    verifyDocument(document);
}

TEST(SPTK_XmlDocument, add)
{
    xml::Document document;
    document.load(testXML);

    document.add<xml::Element>("name")->text("John");
    document.add<xml::Element>("age")->text("33");
    document.add<xml::Element>("temperature")->text("33.6");
    document.add<xml::Element>("timestamp")->text("1519005758000");

    auto* skills = new xml::Element(&document, "skills");
    skills->add<xml::Element>("skill")->text("C++");
    skills->add<xml::Element>("skill")->text("Java");
    skills->add<xml::Element>("skill")->text("Motorbike");

    auto* address = new xml::Element(&document, "address");
    address->add<xml::Element>("married")->text("true");
    address->add<xml::Element>("employed")->text("false");

    verifyDocument(document);
}

TEST(SPTK_XmlDocument, remove)
{
    xml::Document document;
    document.load(testXML);

    document.remove(document.findOrCreate("name"));
    document.remove(document.findOrCreate("age"));
    document.remove(document.findOrCreate("skills"));
    document.remove(document.findOrCreate("address"));
    EXPECT_TRUE(document.findFirst("name") == nullptr);
    EXPECT_TRUE(document.findFirst("age") == nullptr);
    EXPECT_TRUE(document.findFirst("temperature") != nullptr);
    EXPECT_TRUE(document.findFirst("skills") == nullptr);
    EXPECT_TRUE(document.findFirst("address") == nullptr);
}

TEST(SPTK_XmlDocument, save1)
{
    xml::Document document;
    document.load(testREST);

    Buffer buffer;

    document.indentSpaces(0);
    document.save(buffer, 0);

    EXPECT_STREQ(testREST.c_str(), buffer.c_str());
}

TEST(SPTK_XmlDocument, save2)
{
    xml::Document document;
    document.load(testXML);

    Buffer buffer;
    document.save(buffer, 0);

    document.load(testXML);
    verifyDocument(document);
}

TEST(SPTK_XmlDocument, parse)
{
    xml::Document document;
    document.load(testREST);

    const auto* xmlElement = document.findFirst("xml");
    EXPECT_STREQ(xmlElement->getAttribute("version").asString().c_str(), "1.0");
    EXPECT_STREQ(xmlElement->getAttribute("encoding").asString().c_str(), "UTF-8");

    const auto* bodyElement = document.findFirst("soap:Body");
    if (bodyElement == nullptr)
        FAIL() << "Node soap:Body not found";
    EXPECT_EQ(xml::Node::Type::DOM_ELEMENT, bodyElement->type());
    EXPECT_EQ(1, (int) bodyElement->size());
    EXPECT_STREQ("soap:Body", bodyElement->name().c_str());

    const xml::Node* methodElement = nullptr;
    for (const auto* node: *bodyElement)
    {
        if (node->isElement())
        {
            methodElement = node;
            break;
        }
    }
    EXPECT_EQ(xml::Node::Type::DOM_ELEMENT, methodElement->type());
    EXPECT_EQ(2, (int) methodElement->size());
    EXPECT_STREQ("ns1:GetRequests", methodElement->name().c_str());
}

TEST(SPTK_XmlDocument, brokenXML)
{
    xml::Document document;

    try
    {
        const String brokenXML1("<xml></html>");
        document.load(brokenXML1);
        FAIL() << "Must throw exception";
    }
    catch (const Exception& e)
    {
        SUCCEED() << "Correct exception: " << e.what();
    }

    try
    {
        const String brokenXML1("<xml><html></xml></html>");
        document.load(brokenXML1);
        FAIL() << "Must throw exception";
    }
    catch (const Exception& e)
    {
        SUCCEED() << "Correct exception: " << e.what();
    }

    try
    {
        const String brokenXML1("<xml</html>");
        document.load(brokenXML1);
        FAIL() << "Must throw exception";
    }
    catch (const Exception& e)
    {
        SUCCEED() << "Correct exception: " << e.what();
    }
}

TEST(SPTK_XmlDocument, unicodeAndSpacesXML)
{
    xml::Document document;

    try
    {
        const String unicodeXML(R"(<?xml version="1.0" encoding="UTF-8"?><p> “Add” </p><span> </span>)");
        document.load(unicodeXML, true);
        Buffer buffer;
        document.save(buffer, 0);
        EXPECT_STREQ(unicodeXML.c_str(), buffer.c_str());
    }
    catch (const Exception& e)
    {
        FAIL() << e.what();
    }
}

TEST(SPTK_XmlDocument, exportToJSON)
{
    xml::Document xmlDocument;
    json::Document jsonDocument;
    xmlDocument.load("<d>" + testXML + "</d>");
    xmlDocument.exportTo(jsonDocument.root());

    Buffer buffer;
    jsonDocument.exportTo(buffer, true);
    COUT(buffer.c_str() << endl)
}

#endif
