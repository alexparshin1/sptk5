/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __XML_DOCUMENT_H__
#define __XML_DOCUMENT_H__

#include <sptk5/xml/Node.h>
#include <sptk5/xml/DocType.h>
#include <sptk5/xml/Element.h>
#include <sptk5/Buffer.h>
#include <sptk5/RegularExpression.h>

#include <string>
#include <map>

namespace sptk
{

/**
 * @addtogroup XML
 * @{
 */

namespace json { class Document; }

namespace xml {
/**
 * XML document.
 *
 * Represents the entire XML document.
 * It provides access to document root node, which includes all nodes in XML document tree.
 */
class SP_EXPORT Document : public Element
{
    friend class Node;

public:

    /**
     * Constructs an empty document, without doctype.
     */
    Document();

    Document(const Document&) = delete;

    /**
     * Constructs a document from XML string
     * @param xml               XML string
     */
    explicit Document(const String& xml);

    /**
     * Constructs an empty document, with doctype.
     * @param name              Name of the document.
     * @param public_id         Public id of the document, placed on DOCTYPE declaration
     * @param system_id         System id of the document, placed on DOCTYPE declaration
     */
    Document(const char* name, const char* public_id, const char* system_id);

    /**
     * Destructor
     */
    ~Document() override
    {
        Document::clear();
    }

    /**
     * Returns node type
     */
    NodeType type() const override
    {
        return DOM_DOCUMENT;
    }

    /**
     * Returns the node name.
     */
    String name() const override;

    /**
     * Sets the new name for the node
     * @param name              New node name
     */
    void name(const String& name) override
    {
        // Document has no node name
    }

    /**
     * Return doctype of document.
     */
    DocType& docType()
    {
        return m_doctype;
    }

    /**
     * Return doctype of document.
     */
    const DocType& docType() const
    {
        return m_doctype;
    }

    /**
     * Return pointer to root element of document
     */
    Node* rootNode();

    /**
     * Return indentation in save
     */
    int indentSpaces() const
    {
        return m_indentSpaces;
    }

    /**
     * Set indentation in save, defaults to 2
     *
     * @param i                 New indent spaces
     */
    void indentSpaces(int i)
    {
        m_indentSpaces = i;
    }

    /**
     * Load document from buffer.
     * @param buffer            Source buffer
     */
    virtual void load(const char* buffer);

    /**
     * Load document from std::string.
     * @param str               Source string
     */
    virtual void load(const std::string& str)
    {
        load(str.c_str());
    }

    /**
     * Load document from buffer.
     * @param buffer            Source buffer
     */
    virtual void load(const Buffer& buffer)
    {
        load(buffer.c_str());
    }

    /**
     * Save document to buffer.
     * @param buffer            Buffer to save document
     * @param indent            Current indent, ignored (always 0)
     */
    void save(Buffer& buffer, int indent) const override;

    /**
     * Save document to JSON element.
     * @param json              JSON element
     */
    void exportTo(json::Element& json) const override;

    /**
     * Does string match a number?
     * @return true if string constains a number
     */
    static bool isNumber(const String& str);

protected:

    /**
     * Creates new named node of type xml::Node::DOM_ELEMENT.
     * It can be added to document DOM tree.
     * @param tagname           Name of the element
     * @see xml::Node
     */
    Node* createElement(const char* tagname);

    /**
     * Extract entities
     * @param docTypeSection    Document type section
     */
    void extractEntities(char* docTypeSection);

    unsigned char* skipSpaces(unsigned char* start) const;

private:

    DocType             m_doctype;                                  ///< Document type
    int                 m_indentSpaces {2};                         ///< Indent spaces
    Buffer              m_encodeBuffer;                             ///< Buffer to encode entities
    Buffer              m_decodeBuffer;                             ///< Decode and encode buffer

    /**
     * Internal entities parser
     */
    void parseEntities(char* entitiesSection);

    /**
     * Internal doctype parser
     */
    void parseDocType(char* docTypeSection);

    /**
     * Internal attributes parser
     */
    void processAttributes(Node* node, const char* ptr);
    static char* readComment(Node* currentNode, char* nodeName, char* nodeEnd, char* tokenEnd);
    static char* readCDataSection(Node* currentNode, char* nodeName, char* nodeEnd, char* tokenEnd);

    char* readDocType(char* tokenEnd);

    RegularExpression   m_parseAttributes { R"(([\w\-_\.:]+)\s*=\s*['"]([^'"]+)['"])","g" };

    char* readExclamationTag(char* nodeName, char* tokenEnd, char* nodeEnd, Node* currentNode);

    char* readProcessingInstructions(const char* nodeName, char* tokenEnd, char*& nodeEnd, Node* currentNode);

    static char* readClosingTag(const char* nodeName, char* tokenEnd, char*& nodeEnd, Node*& currentNode);

    char* readOpenningTag(const char* nodeName, char* tokenEnd, char*& nodeEnd, Node*& currentNode);
};

} // namespace xml

/**
 * @}
 */
}
#endif
