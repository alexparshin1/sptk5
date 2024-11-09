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
    const auto matches = parseAttributes.m(ptr);

    for (auto itor = matches.groups().begin(); itor != matches.groups().end(); itor += 2)
    {
        auto vtor = itor + 1;
        if (vtor == matches.groups().end())
        {
            throw Exception("Invalid attribute format for " + node.getQualifiedName() + " tag");
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
                throw Exception("Invalid attribute format for " + node.getQualifiedName() + " tag");
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
    const char    ch = *tokenEnd;
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
        pi->setName(nodeName + 1);
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
    const char ch = *tokenEnd;
    *tokenEnd = 0;
    if (ch != '>')
    {
        throw Exception("Invalid tag (spaces before closing '>')");
    }
    ++nodeName;
    if (currentNode->getQualifiedName() != nodeName)
    {
        throw Exception(
            "Closing tag <" + string(nodeName) + "> doesn't match opening <" + currentNode->getQualifiedName() + ">");
    }

    nodeEnd = tokenEnd;

    return tokenEnd;
}

char* ImportXML::readOpeningTag(SNode& currentNode, const char* nodeName, char* tokenEnd, char*& nodeEnd)
{
    const char ch = *tokenEnd;
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

    if (const auto len = nodeEnd - tokenStart;
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
    bool   first = true;
    String itemName;
    for (const auto& node: _node->nodes())
    {
        if (first)
        {
            first = false;
            itemName = node->getQualifiedName();
        }
        else
        {
            if (itemName != node->getQualifiedName())
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
    SNode       currentNode = node;
    XMLDocType* doctype = &docType();
    Buffer      buffer(_buffer);

    for (char* nodeStart = strchr(bit_cast<char*>(buffer.data()), '<'); nodeStart != nullptr;)
    {
        auto* nameStart = nodeStart + 1;
        char* nameEnd = strpbrk(nameStart + 1, "?/\r\n <>");
        if (nameEnd == nullptr || *nameEnd == '<')
        {
            throw Exception("Tag started but not closed");
        }

        char* nodeName = nameStart;
        char* nodeEnd = nameStart;
        bool  autoClosed = false;
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
                readOpeningTag(currentNode, nodeName, nameEnd, nodeEnd);
                // For HTML, autoclose 'meta' tags
                if (strcmp(nodeName, "meta") == 0 && currentNode->parent()->getQualifiedName().in({"html", "head"}))
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
                const auto skipSpaces = strspn(textStart, "\n\r\t ");
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
        doctype->decodeEntities(textStart, static_cast<uint32_t>(textTrail - textStart), decoded);
        String decodedText(decoded.c_str(), decoded.size());

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

        if (nodeType != Node::Type::Number && formatting == Mode::KeepFormatting)
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

} // namespace sptk::xdoc
