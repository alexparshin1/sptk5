/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Document.cpp - description                             ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <cstdlib>
#include <sptk5/Strings.h>
#include <sptk5/cxml>
#include <sptk5/json/JsonDocument.h>

using namespace std;
using namespace sptk;

namespace sptk {
namespace xml {

static const char* MATCH_NUMBER = "^[+\\-]?(\\d|[1-9]\\d*)(\\.\\d+)?(e-?\\d+)?$";

Document::Document()
        :
        Element(*this),
        m_indentSpaces(2),
        m_matchNumber(MATCH_NUMBER, "i")
{
}

Document::Document(const String& xml)
        :
        Element(*this),
        m_indentSpaces(2),
        m_matchNumber(MATCH_NUMBER, "i")
{
    load(xml);
}

Document::Document(const char* aname, const char* public_id, const char* system_id)
: Element(*this),
  m_doctype(aname, public_id, system_id),
  m_indentSpaces(2),
  m_matchNumber(MATCH_NUMBER, "i")
{
}

Node* Document::rootNode()
{
    for (auto nd: *this) {
        if (nd->type() == DOM_ELEMENT)
            return nd;
    }
    return nullptr;
}

void Document::clear()
{
    Element::clear();
    SharedStrings::clear();
}

Node* Document::createElement(const char* tagname)
{
    Node* node = new Element(this, tagname);
    return node;
}

void Document::processAttributes(Node* node, const char* ptr)
{
    const char* tokenStart = ptr;

    Attributes& attr = node->attributes();
    while (*tokenStart != 0 && *tokenStart <= ' ')
        tokenStart++;

    while (*tokenStart != 0) {
        auto* tokenEnd = (char*) strpbrk(tokenStart, " =");
        if (tokenEnd == nullptr)
            throw Exception("Incorrect attribute - missing '='");
        *tokenEnd = 0;

        const char* attributeName = tokenStart;
        char* attributeValue = tokenEnd + 1;
        while (*attributeValue == ' ' || *attributeValue == '=')
            attributeValue++;
        char delimiter = *attributeValue;
        uint32_t valueLength;
        if (delimiter == '\'' || delimiter == '\"') {
            attributeValue++;
            tokenEnd = strchr(attributeValue, delimiter);
            if (tokenEnd == nullptr)
                throw Exception("Incorrect attribute format - missing quote");
            *tokenEnd = 0;
            valueLength = uint32_t(tokenEnd - attributeValue);
        } else {
            tokenEnd = strchr(attributeValue, ' ');
            if (tokenEnd != nullptr) {
                valueLength = uint32_t(tokenEnd - attributeValue);
                *tokenEnd = 0;
            } else {
                valueLength = (uint32_t) strlen(attributeValue);
            }
        }

        m_encodeBuffer.bytes(0);
        m_doctype.decodeEntities(attributeValue, valueLength, m_encodeBuffer);
        attr.setAttribute(attributeName, m_encodeBuffer.c_str());

        if (tokenEnd != nullptr)
            tokenStart = tokenEnd + 1;
        else
            tokenStart = "";

        while (*tokenStart != 0 && *tokenStart <= ' ')
            tokenStart++;
    }
}

void Document::parseEntities(char* entitiesSection)
{
    auto* start = (unsigned char*) entitiesSection;
    while (start != nullptr) {
        start = (unsigned char*) strstr((char*) start, "<!ENTITY ");
        if (start == nullptr)
            break;
        start += 9;
        while (*start <= ' ')
            start++;
        auto* end = (unsigned char*) strchr((char*) start, ' ');
        if (end == nullptr)
            break;
        *end = 0;
        unsigned char* ent_name = start;
        unsigned char* ent_value = end + 1;
        while (*ent_value <= ' ')
            ent_value++;
        unsigned char delimiter = *ent_value;
        if (delimiter == '\'' || delimiter == '\"') {
            ent_value++;
            end = (unsigned char*) strchr((char*) ent_value, (char) delimiter);
            if (end == nullptr)
                break;
            *end = 0;
        } else {
            end = (unsigned char*) strpbrk((char*) ent_value, " >");
            if (end == nullptr)
                break;
            if (*end == ' ') {
                *end = 0;
                end = (unsigned char*) strchr((char*) ent_value, '>');
                if (end == nullptr)
                    break;
            }
            *end = 0;
        }
        m_doctype.m_entities.setEntity((char*) ent_name, (char*) ent_value);
        start = end + 1;
    }
}

void Document::parseDocType(char* docTypeSection)
{
    m_doctype.m_name = "";
    m_doctype.m_public_id = "";
    m_doctype.m_system_id = "";
    m_doctype.m_entities.clear();
    char* start = docTypeSection;
    int index = 0;
    //uint32_t len = strlen(docTypeSection);
    int t = 0;
    char* entitiesSection = strchr(docTypeSection, '[');
    if (entitiesSection != nullptr) {
        *entitiesSection = 0;
        entitiesSection++;
        char* end = strchr(entitiesSection, ']');
        if (end != nullptr) {
            *end = 0;
            parseEntities(entitiesSection);
        }
    }
    char delimiter = ' ';
    while (start != nullptr) {
        while (*start == ' ' || *start == delimiter)
            start++;
        char* end = strchr(start, delimiter);
        if (end != nullptr)
            *end = 0;
        switch (index) {
            case 0:
                m_doctype.m_name = start;
                if (end == nullptr)
                    return;
                break;
            case 1:
            case 3:
                if (end == nullptr)
                    break;
                if (strcmp(start, "SYSTEM") == 0) {
                    t = 0;
                } else if (strcmp(start, "PUBLIC") == 0) {
                    t = 1;
                }
                delimiter = '\"';
                break;
            case 2:
            case 4:
                switch (t) {
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
            break;
        start = end + 1;
        index++;
    }
}

void Document::load(const char* xmlData)
{
    clear();
    Node* currentNode = this;
    DocType* doctype = &docType();
    char* buffer = strdup(xmlData);
    try {
        char* tokenStart = strchr(buffer, '<');
        while (tokenStart != nullptr) {
            tokenStart++;
            char* tokenEnd = strpbrk(tokenStart, "\r\n >");
            if (tokenEnd == nullptr)
                break; /// Tag started but not completed

            char ch = *tokenEnd;
            *tokenEnd = 0;
            char* nodeName = tokenStart;
            char* nodeEnd;
            switch (*tokenStart) {
                case '!':
                    if (strncmp(nodeName, "!--", 3) == 0) {
                        /// Comment
                        *tokenEnd = ch; // ' ' or '>' could be within a comment
                        nodeEnd = strstr(nodeName + 3, "-->");
                        if (nodeEnd == nullptr)
                            throw Exception("Invalid end of the comment tag");
                        *nodeEnd = 0;
                        new Comment(currentNode, nodeName + 3);
                        tokenEnd = nodeEnd + 2;
                        break;
                    }
                    if (strncmp(nodeName, "![CDATA[", 8) == 0) {
                        /// CDATA section
                        *tokenEnd = ch;
                        nodeEnd = strstr(nodeName + 1, "]]>");
                        if (nodeEnd == nullptr)
                            throw Exception("Invalid CDATA section");
                        *nodeEnd = 0;
                        new CDataSection(currentNode, nodeName + 8);
                        tokenEnd = nodeEnd + 2;
                        break;
                    }
                    if (strncmp(nodeName, "!DOCTYPE", 8) == 0) {
                        /// DOCTYPE section
                        if (ch == '>')
                            break;
                        nodeEnd = (char*) strstr(tokenEnd + 1, "]>");
                        if (nodeEnd != nullptr) { /// ENTITIES
                            nodeEnd++;
                            *nodeEnd = 0;
                        } else {
                            nodeEnd = strchr(tokenEnd + 1, '>');
                            if (nodeEnd == nullptr)
                                throw Exception("Invalid CDATA section");
                            *nodeEnd = 0;
                        }
                        parseDocType(tokenEnd + 1);
                        tokenEnd = nodeEnd;
                    }
                    break;

                case '?': {
                    /// Processing instructions
                    const char* value;
                    if (ch == ' ') {
                        value = tokenEnd + 1;
                        nodeEnd = (char*) strstr(value, "?>");
                    } else {
                        value = "";
                        nodeEnd = strstr(tokenStart, "?");
                    }
                    if (nodeEnd == nullptr)
                        throw Exception("Invalid PI section");
                    *nodeEnd = 0;
                    new PI(currentNode, nodeName + 1, value);
                    tokenEnd = nodeEnd + 1;
                }
                    break;

                case '/':
                    /// Closing tag
                    if (ch != '>')
                        throw Exception("Invalid tag (spaces before closing '>')");
                    nodeName++;
                    if (currentNode->name() != nodeName)
                        throw Exception(
                                "Closing tag <" + string(nodeName) + "> doesn't match opening <" + currentNode->name() +
                                ">");
                    currentNode = currentNode->parent();
                    if (currentNode == nullptr)
                        throw Exception(
                                "Closing tag <" + string(nodeName) + "> doesn't have corresponding opening tag");
                    break;

                default:
                    /// Normal tag
                    if (ch == '>') {
                        if (*(tokenEnd - 1) == '/') {
                            *(tokenEnd - 1) = 0;
                            new Element(currentNode, nodeName);
                        } else
                            currentNode = new Element(currentNode, nodeName);
                        break;
                    }

                    /// Attributes
                    tokenStart = tokenEnd + 1;
                    nodeEnd = strchr(tokenStart, '>');
                    if (nodeEnd == nullptr)
                        throw Exception("Invalid tag (started, not closed)");
                    *nodeEnd = 0;
                    Node* anode;
                    if (*(nodeEnd - 1) == '/') {
                        anode = new Element(currentNode, nodeName);
                        *(nodeEnd - 1) = 0;
                    } else {
                        anode = currentNode = new Element(currentNode, nodeName);
                    }
                    processAttributes(anode, tokenStart);
                    tokenEnd = nodeEnd;
                    break;
            }
            tokenStart = strchr(tokenEnd + 1, '<');
            if (tokenStart == nullptr) {
                if (currentNode == this)
                    break;
                throw Exception("Tag started but not closed");
            }
            unsigned char* textStart = (unsigned char*) tokenEnd + 1;
            while (*textStart <= ' ') /// Skip leading spaces
                textStart++;
            if (*textStart != '<')
                for (unsigned char* textTrail = (unsigned char*) tokenStart - 1; textTrail >= textStart; textTrail--) {
                    if (*textTrail > ' ') {
                        textTrail++;
                        *textTrail = 0;
                        Buffer& decoded = m_decodeBuffer;
                        doctype->decodeEntities((char*) textStart, uint32_t(textTrail - textStart), decoded);
                        new Text(currentNode, decoded.c_str());
                        break;
                    }
                }
        }
    }
    catch (...) {
        free(buffer);
        throw;
    }
    free(buffer);
}

void Document::save(Buffer& buffer, int) const
{
    Node* xml_pi = nullptr;

    buffer.reset();

    // Write XML PI
    bool hasXmlPI = false;
    for (auto node: *this) {
        if (node->type() == DOM_PI && lowerCase(node->name()) == "xml") {
            xml_pi = node;
            xml_pi->save(buffer);
            hasXmlPI = true;
            break;
        }
    }
    if (!hasXmlPI)
        buffer.append("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");

    if (!docType().name().empty()) {
        buffer.append("<!DOCTYPE " + docType().name());
        if (!docType().systemID().empty())
            buffer.append(" SYSTEM \"" + docType().systemID() + "\"");

        if (!docType().publicID().empty())
            buffer.append(" PUBLIC \"" + docType().publicID() + "\"");

        if (!docType().entities().empty()) {
            buffer.append(" [\n", 3);
            const Entities& entities = docType().entities();
            for (auto it: entities)
                buffer.append("<!ENTITY " + it.first + " \"" + it.second + "\">\n");
            buffer.append("]", 1);
        }
        buffer.append(">\n", 2);
    }

    // call save() method of the first (and hopefully only) node in xml document
    for (auto node: *this) {
        if (node == xml_pi)
            continue;
        node->save(buffer, 0);
    }
}

const std::string& Document::name() const
{
    static const string nodeNameString("#document");
    return nodeNameString;
}

void Document::exportTo(json::Element& json) const
{
    Node* rootNode = *begin();
    rootNode->exportTo(json);
    json.optimizeArrays("item");
}

} // namespace xml
} // namespace sptk

#if USE_GTEST
#include <gtest/gtest.h>

static const char* testXML =
        "<name>John</name><age>33</age><temperature>33.6</temperature><timestamp>1519005758000</timestamp>"
        "<skills><skill>C++</skill><skill>Java</skill><skill>Motorbike</skill></skills>"
        "<address><married>true</married><employed>false</employed></address>";

void verifyDocument(xml::Document& document)
{
    EXPECT_STREQ("John", document.findOrCreate("name")->text().c_str());

    EXPECT_EQ(33, string2int(document.findOrCreate("age")->text()));
    EXPECT_DOUBLE_EQ(33.6, string2double(document.findOrCreate("temperature")->text()));
    EXPECT_DOUBLE_EQ(1519005758, int(string2int64(document.findOrCreate("timestamp")->text())/1000));

    Strings skills;
    for (auto node: *document.findOrCreate("skills")) {
        skills.push_back(node->text());
    }
    EXPECT_STREQ("C++,Java,Motorbike", skills.join(",").c_str());

    xml::Node* ptr = document.findFirst("address");
    EXPECT_TRUE(ptr != nullptr);
    auto* ptr2 = dynamic_cast<xml::Element*>(ptr);

    xml::Element& address = *ptr2;
    EXPECT_STREQ("true", address.findOrCreate("married")->text().c_str());
    EXPECT_STREQ("false", address.findOrCreate("employed")->text().c_str());
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

    (new xml::Element(&document, "name"))->text("John");
    (new xml::Element(&document, "age"))->text("33");
    (new xml::Element(&document, "temperature"))->text("33.6");
    (new xml::Element(&document, "timestamp"))->text("1519005758000");

    auto skills = new xml::Element(&document, "skills");
    (new xml::Element(skills, "skill"))->text("C++");
    (new xml::Element(skills, "skill"))->text("Java");
    (new xml::Element(skills, "skill"))->text("Motorbike");

    auto address = new xml::Element(&document, "address");
    (new xml::Element(address, "married"))->text("true");
    (new xml::Element(address, "employed"))->text("false");

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

TEST(SPTK_XmlDocument, save)
{
    xml::Document document;
    document.load(testXML);

    Buffer buffer;
    document.save(buffer, 0);

    document.load(testXML);
    verifyDocument(document);
}

#endif
