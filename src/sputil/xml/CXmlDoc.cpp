/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CXmlDoc.cpp  -  description
                             -------------------
    begin                : Sun May 22 2003
    based on the code    : Mikko Lahteenmaki <Laza@Flashmail.com>
    copyright            : (C) 1999-2013 by Alexey S.Parshin
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <ctype.h>
#include <stdlib.h>

#include <sptk5/CStrings.h>
#include <sptk5/cxml>
#include <iostream>

using namespace std;
using namespace sptk;

CXmlDoc::CXmlDoc() :
    CXmlElement(*this),
    m_indentSpaces(2)
{
}

CXmlDoc::CXmlDoc(const char *aname, const char *public_id, const char *system_id) :
    CXmlElement(*this),
    m_doctype(aname, public_id, system_id),
    m_indentSpaces(2)
{
}

CXmlNode *CXmlDoc::rootNode()
{
    iterator itor = begin();
    iterator iend = end();
    for (; itor != iend; itor++) {
        CXmlNode *nd = *itor;
        if (nd->type() == DOM_ELEMENT)
            return nd;
    }
    return 0;
}

void CXmlDoc::clear()
{
    CXmlElement::clear();
    CSharedStrings::clear();
}

CXmlNode *CXmlDoc::createElement(const char *tagname)
{
    CXmlNode *node = new CXmlElement(this, tagname);
    return node;
}

void CXmlDoc::processAttributes(CXmlNode* node, char *ptr)
{
    const char* tokenStart = ptr;

    CXmlAttributes& attr = node->attributes();
    while (*tokenStart == ' ')
        tokenStart++;
    while (*tokenStart) {
        char* tokenEnd = (char*) strpbrk(tokenStart, " =");
        if (!tokenEnd)
            throw CException("Incorrect attribute - missing '='");
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
            if (!tokenEnd)
                throw CException("Incorrect attribute format - missing quote");
            *tokenEnd = 0;
            valueLength = uint32_t(tokenEnd - attributeValue);
        } else {
            tokenEnd = strchr(attributeValue, ' ');
            if (tokenEnd) {
                valueLength = uint32_t(tokenEnd - attributeValue);
                *tokenEnd = 0;
            } else {
                valueLength = (uint32_t) strlen(attributeValue);
            }
        }

        m_encodeBuffer.bytes(0);
        m_doctype.decodeEntities(attributeValue, valueLength, m_encodeBuffer);
        attr.setAttribute(attributeName, m_encodeBuffer.c_str());

        if (tokenEnd)
            tokenStart = tokenEnd + 1;
        else
            tokenStart = "";

        while (*tokenStart == ' ')
            tokenStart++;
    }
}

void CXmlDoc::parseEntities(char* entitiesSection)
{
    unsigned char* start = (unsigned char*) entitiesSection;
    while (start) {
        start = (unsigned char*) strstr((char*) start, "<!ENTITY ");
        if (!start)
            break;
        start += 9;
        while (*start <= ' ')
            start++;
        unsigned char* end = (unsigned char*) strchr((char*) start, ' ');
        if (!end)
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
            if (!end)
                break;
            *end = 0;
        } else {
            end = (unsigned char*) strpbrk((char*) ent_value, " >");
            if (!end)
                break;
            if (*end == ' ') {
                *end = 0;
                end = (unsigned char*) strchr((char*) ent_value, '>');
                if (!end)
                    break;
            }
            *end = 0;
        }
        m_doctype.m_entities.setEntity((char*) ent_name, (char*) ent_value);
        start = end + 1;
    }
}

void CXmlDoc::parseDocType(char* docTypeSection)
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
    if (entitiesSection) {
        *entitiesSection = 0;
        entitiesSection++;
        char* end = strchr(entitiesSection, ']');
        if (end) {
            *end = 0;
            parseEntities(entitiesSection);
        }
    }
    char delimiter = ' ';
    while (start) {
        while (*start == ' ' || *start == delimiter)
            start++;
        char* end = strchr(start, delimiter);
        if (end)
            *end = 0;
        switch (index)
        {
        case 0:
            m_doctype.m_name = start;
            if (!end)
                return;
            break;
        case 1:
        case 3:
            if (!end)
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
            if (t == 0) {
                m_doctype.m_system_id = start;
                break;
            }
            if (t == 1) {
                m_doctype.m_public_id = start;
                break;
            }
            delimiter = ' ';
            break;
        }
        if (!end)
            break;
        start = end + 1;
        index++;
    }
}

void CXmlDoc::load(const char* xmlData)
{
    clear();
    CXmlNode* currentNode = this;
    CXmlDocType *doctype = &docType();
    char* buffer = strdup(xmlData);
    try {
        char* tokenStart = (char*) strchr(buffer, '<');
        while (tokenStart) {
            tokenStart++;
            char* tokenEnd = strpbrk(tokenStart, " >");
            if (!tokenEnd)
                break; /// Tag started but not completed

            char ch = *tokenEnd;
            *tokenEnd = 0;
            char* nodeName = tokenStart;
            char* nodeEnd;
            switch (*tokenStart)
            {
            case '!':
                if (strncmp(nodeName, "!--", 3) == 0) {
                    /// Comment
                    *tokenEnd = ch; // ' ' or '>' could be within a comment
                    nodeEnd = strstr(nodeName + 3, "-->");
                    if (!nodeEnd)
                        throw CException("Invalid end of the comment tag");
                    *nodeEnd = 0;
                    new CXmlComment(currentNode, nodeName + 3);
                    tokenEnd = nodeEnd + 2;
                    break;
                }
                if (strncmp(nodeName, "![CDATA[", 8) == 0) {
                    /// CDATA section
                    if (ch != '>') {
                        nodeEnd = strstr(tokenEnd + 1, "]]>");
                        *tokenEnd = ch;
                    } else
                        nodeEnd = strstr(nodeName + 8, "]]");
                    if (!nodeEnd)
                        throw CException("Invalid CDATA section");
                    *nodeEnd = 0;
                    new CXmlCDataSection(currentNode, nodeName + 8);
                    tokenEnd = nodeEnd + 3;
                    break;
                }
                if (strncmp(nodeName, "!DOCTYPE", 8) == 0) {
                    /// DOCTYPE section
                    if (ch == '>')
                        break;
                    nodeEnd = strstr(tokenEnd + 1, "]>");
                    if (nodeEnd) { /// ENTITIES
                        nodeEnd++;
                        *nodeEnd = 0;
                    } else {
                        nodeEnd = strchr(tokenEnd + 1, '>');
                        if (!nodeEnd)
                            throw CException("Invalid CDATA section");
                        *nodeEnd = 0;
                    }
                    parseDocType(tokenEnd + 1);
                    tokenEnd = nodeEnd;
                }
                break;

            case '?': {
                /// Processing instructions
                const char *value;
                if (ch == ' ') {
                    value = tokenEnd + 1;
                    nodeEnd = (char*) strstr(value, "?>");
                } else {
                    value = "";
                    nodeEnd = strstr(tokenStart, "?");
                }
                if (!nodeEnd)
                    throw CException("Invalid PI section");
                *nodeEnd = 0;
                new CXmlPI(currentNode, nodeName + 1, value);
                tokenEnd = nodeEnd + 2;
            }
                break;

            case '/':
                /// Closing tag
                if (ch != '>')
                    throw CException("Invalid tag (spaces before closing '>')");
                nodeName++;
                if (currentNode->name() != nodeName)
                    throw CException(
                            "Closing tag <" + string(nodeName) + "> doesn't match opening <" + currentNode->name() + ">");
                currentNode = currentNode->parent();
                if (!currentNode)
                    throw CException("Closing tag <" + string(nodeName) + "> doesn't have corresponding opening tag");
                break;

            default:
                /// Normal tag
                if (ch == '>') {
                    if (*(tokenEnd - 1) == '/') {
                        *(tokenEnd - 1) = 0;
                        new CXmlElement(currentNode, nodeName);
                    } else
                        currentNode = new CXmlElement(currentNode, nodeName);
                    break;
                }

                /// Attributes
                tokenStart = tokenEnd + 1;
                nodeEnd = strchr(tokenStart, '>');
                if (!nodeEnd)
                    throw CException("Invalid tag (started, not closed)");
                *nodeEnd = 0;
                CXmlNode* anode;
                if (*(nodeEnd - 1) == '/') {
                    anode = new CXmlElement(currentNode, nodeName);
                    *(nodeEnd - 1) = 0;
                } else {
                    anode = currentNode = new CXmlElement(currentNode, nodeName);
                }
                processAttributes(anode, tokenStart);
                tokenEnd = nodeEnd;
                break;
            }
            tokenStart = strchr(tokenEnd + 1, '<');
            if (!tokenStart) {
                if (currentNode == this)
                    return;
                throw CException("Tag started but not closed");
            }
            unsigned char* textStart = (unsigned char*) tokenEnd + 1;
            while (*textStart <= ' ') /// Skip leading spaces
                textStart++;
            if (*textStart != '<')
                for (unsigned char* textTrail = (unsigned char*) tokenStart - 1; textTrail >= textStart; textTrail--) {
                    if (*textTrail > ' ') {
                        textTrail++;
                        *textTrail = 0;
                        CBuffer& decoded = m_decodeBuffer;
                        doctype->decodeEntities((char*) textStart, uint32_t(textTrail - textStart), decoded);
                        new CXmlText(currentNode, decoded.c_str());
                        break;
                    }
                }
        }
    } catch (...) {
        free(buffer);
        throw;
    }
}

void CXmlDoc::save(CBuffer &buffer, int /*indent*/) const
{
    CXmlNode *xml_pi = 0;

    buffer.reset();

    // Write XML PI
    const_iterator itor = begin();
    const_iterator iend = end();
    for (; itor != iend; itor++) {
        CXmlNode *node = *itor;
        if (node->type() == DOM_PI && node->name() == "XML") {
            xml_pi = node;
            xml_pi->save(buffer);
        }
    }

    if (!docType().name().empty()) {
        buffer.append("<!DOCTYPE " + docType().name());
        if (!docType().systemID().empty())
            buffer.append(" SYSTEM \"" + docType().systemID() + "\"");

        if (!docType().publicID().empty())
            buffer.append(" PUBLIC \"" + docType().publicID() + "\"");

        if (docType().entities().size() > 0) {
            buffer.append(" [\n", 3);
            const CXmlEntities& entities = docType().entities();
            CXmlEntities::const_iterator it = entities.begin();
            for (; it != entities.end(); it++) {
                buffer.append("<!ENTITY " + it->first + " \"" + it->second + "\">\n");
            }
            buffer.append("]", 1);
        }
        buffer.append(">\n", 2);
    }

    // call save() method of the first (and hopefully only) node in xmldocument
    for (itor = begin(); itor != iend; itor++) {
        CXmlNode *node = *itor;
        if (node == xml_pi)
            continue;
        node->save(buffer, 0);
    }
}

const std::string& CXmlDoc::name() const
{
    static const string nodeNameString("#document");
    return nodeNameString;
}
