/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include <sptk5/Buffer.h>
#include <sptk5/RegularExpression.h>
#include <sptk5/xdoc/Node.h>
#include <sptk5/xdoc/XMLDocType.h>

#include <map>
#include <string>

namespace sptk::xdoc {

/**
 * @addtogroup XDoc.
 * @{
 */

/**
 * @brief XML document.
 *
 * Represents the entire XML document.
 * It provides access to document root node, which includes all nodes in XML document tree.
 */
class SP_EXPORT ImportXML
{
public:
    enum class Mode : uint8_t
    {
        Compact,       ///< Strip any XML formatting, store #text nodes directly into Nodes.
        KeepFormatting ///< Keep any #text nodes.
    };

    /**
     * @brief Constructs an empty document, without doctype.
     */
    ImportXML() = default;

    virtual ~ImportXML() = default;

    /**
     * @brief Return doctype of document.
     */
    XMLDocType& docType()
    {
        return m_doctype;
    }

    /**
     * @brief Load document from buffer.
     * @param _buffer            Source buffer.
     */
    void parse(const SNode& node, const char* _buffer, Mode formatting = Mode::Compact);

private:
    XMLDocType m_doctype;      ///< XMLDocument type.
    Buffer     m_encodeBuffer; ///< Buffer to encode entities.
    Buffer     m_decodeBuffer; ///< Decode and encode buffer.

    /**
     * @brief Internal attributes parser.
     */
    void processAttributes(Node& node, const char* ptr);

    static char* readComment(const SNode& currentNode, char* nodeName, char* nodeEnd, char* tokenEnd);

    static char* readCDataSection(const SNode& currentNode, char* nodeName, char* nodeEnd, char* tokenEnd,
                                  Mode formatting);

    static char* readXMLDocType(char* tokenEnd);

    static const RegularExpression parseAttributes;

    static char* readExclamationTag(const SNode& currentNode, char* nodeName, char* tokenEnd, char* nodeEnd, Mode formatting);

    char* readProcessingInstructions(const SNode& currentNode, const char* nodeName, char* tokenEnd, char*& nodeEnd,
                                     bool isRootNode);

    char* readOpeningTag(SNode& currentNode, const char* nodeName, char* tokenEnd, char*& nodeEnd);

    static char* readClosingTag(const SNode& currentNode, const char* nodeName, char* tokenEnd, char*& nodeEnd);

    void readText(const SNode& currentNode, XMLDocType* doctype, const char* nodeStart, const char* textStart,
                  Mode formatting);

    static SNode detectArray(const SNode& node);
};

/**
 * @}
 */
} // namespace sptk::xdoc
