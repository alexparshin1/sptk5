/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       XMLDoc.h - description                                 ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_XMLDOCUMENT_H__
#define __SPTK_XMLDOCUMENT_H__

#include <sptk5/xml/XMLNode.h>
#include <sptk5/xml/XMLDocType.h>
#include <sptk5/xml/XMLElement.h>
#include <sptk5/SharedStrings.h>
#include <sptk5/Buffer.h>

#include <string>
#include <map>

namespace sptk
{

/**
 * @addtogroup XML
 * @{
 */

/**
 * @brief XML document.
 *
 * Represents the entire XML document.
 * It provides access to document root node, which includes all nodes in XML document tree.
 */
class SP_EXPORT XMLDocument: public SharedStrings, public XMLElement
{
    friend class XMLNode;

    /**
     * Document type
     */
    XMLDocType m_doctype;

    /**
     * Indent spaces
     */
    int        m_indentSpaces;

    /**
     * Buffer to encode entities
     */
    Buffer     m_encodeBuffer;


    /**
     * @brief Internal entities parser
     */
    void parseEntities(char* entitiesSection);

    /**
     * @brief Internal doctype parser
     */
    void parseDocType(char* docTypeSection);

    /**
     * @brief Internal attributes parser
     */
    void processAttributes(XMLNode* node, const char *ptr);

protected:

    /**
     * Decode and encode buffer
     */
    Buffer m_decodeBuffer;


    /**
     * Creates new named node of type XMLNode::DOM_ELEMENT.
     * It can be added to document DOM tree.
     * @param tagname const char *, name of the element
     * @see XMLNode
     */
    XMLNode *createElement(const char *tagname);

public:

    /**
     * @brief Constructs an empty document, without doctype.
     */
    XMLDocument();

    /**
     * @brief Constructs a document from XML string
     * @param xml std::string, XML string
     */
    XMLDocument(std::string xml);
    
    /**
     * @brief Constructs an empty document, with doctype.
     * @param name const char *, name of the document.
     * @param public_id const char *, public id of the document, placed on DOCTYPE declaration
     * @param system_id const char *, system id of the document, placed on DOCTYPE declaration
     */
    XMLDocument(const char *name, const char *public_id, const char *system_id);

    /**
     * @brief Destructor
     */
    virtual ~XMLDocument()
    {
        clear();
    }

    /**
     * @brief Returns node type
     */
    virtual XMLNodeType type() const override
    {
        return DOM_DOCUMENT;
    }

    /**
     * @brief Destroys all nodes in document
     */
    virtual void clear() override;

    /**
     * @brief Returns the node name.
     */
    virtual const std::string& name() const override;

    /**
     * @brief Sets the new name for the node
     * @param name const std::string&, new node name
     */
    virtual void name(const std::string& name) override
    {
    }

    /**
     * @brief Sets new name for node
     * @param name const char *, new node name
     */
    virtual void name(const char *name) override
    {
    }

    /**
     * @brief Returns doctype of document.
     *
     * You can use it to add e.g. custom entities.
     * <pre>
     * mydoc->doctype().set_entity("myentity", "myreplacement");
     * </pre>
     */
    XMLDocType &docType()
    {
        return m_doctype;
    }

    /**
     * @brief Returns doctype of document.
     *
     * You can use it to add e.g. custom entities.
     * <pre>
     * mydoc->doctype().set_entity("myentity", "myreplacement");
     * </pre>
     */
    const XMLDocType &docType() const
    {
        return m_doctype;
    }

    /**
     * @brief Returns pointer to root element of document
     */
    XMLNode *rootNode();

    /**
     * @brief Returns indentation in save
     */
    int indentSpaces()
    {
        return m_indentSpaces;
    }

    /**
     * @brief Set indentation in save, defaults to 2
     *
     * @param i as new indent spaces
     */
    void indentSpaces(int i)
    {
        m_indentSpaces = i;
    }

    /**
     * @brief Loads document from buffer.
     * @param buffer const char*, source buffer
     */
    virtual void load(const char* buffer);

    /**
     * @brief Loads document from std::string.
     * @param str const std::string&, source string
     */
    virtual void load(const std::string& str)
    {
        load(str.c_str());
    }

    /**
     * @brief Loads document from buffer.
     * @param buffer const CBuffer&, source buffer
     */
    virtual void load(const Buffer& buffer)
    {
        load(buffer.c_str());
    }

    /**
     * @brief Saves document to buffer.
     * @param buffer CBuffer&, a buffer to save document
     * @param formalXML bool, if true then prepend with '<?xml version="1.0" ?>'
     */
    virtual void save(Buffer& buffer, bool formalXML=false) const;

    /**
     * @brief Saves document to buffer.
     * @param buffer CBuffer&, a buffer to save document
     * @param indent int, formatting indent (spaces)
     */
    virtual void save(Buffer& buffer, int indent) const override
    {
        XMLElement::save(buffer, indent);
    }
};
/**
 * @}
 */
}
#endif
