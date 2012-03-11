/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CXmlDoc.h  -  description
                             -------------------
    begin                : Sun May 22 2003
    based on the code    : Mikko Lahteenmaki <Laza@Flashmail.com>
    copyright            : (C) 2003-2012 by Alexey Parshin. All rights reserved.
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

#ifndef __CXMLDOC_H__
#define __CXMLDOC_H__

#include <sptk5/xml/CXmlNode.h>
#include <sptk5/xml/CXmlDocType.h>
#include <sptk5/xml/CXmlElement.h>
#include <sptk5/CSharedStrings.h>
#include <sptk5/CBuffer.h>

#include <string>
#include <map>

namespace sptk
{

/// @addtogroup XML
/// @{

/// @brief XML document.
///
/// Represents the entire XML document.
/// It provides access to document root node, which includes all nodes in XML document tree.
class SP_EXPORT CXmlDoc: public CSharedStrings, public CXmlElement
{
    friend class CXmlNode;

    CXmlDocType m_doctype;          ///< Document type
    int m_indentSpaces;     ///< Indent spaces
    CBuffer m_encodeBuffer;     ///< Buffer to encode entities

    /// @brief Internal entities parser
    void parseEntities(char* entitiesSection);

    /// @brief Internal doctype parser
    void parseDocType(char* docTypeSection);

    /// @brief Internal attributes parser
    void processAttributes(CXmlNode* node, char *ptr);

protected:

    CBuffer m_decodeBuffer;     ///< Decode and encode buffer

    /// Creates new named node of type CXmlNode::DOM_ELEMENT.
    /// It can be added to document DOM tree.
    /// @param tagname const char *, name of the element
    /// @see CXmlNode
    CXmlNode *createElement(const char *tagname);

public:

    /// @brief Constructs an empty document, without doctype.
    CXmlDoc();

    /// @brief Constructs an empty document, with doctype.
    /// @param name const char *, name of the document.
    /// @param public_id const char *, public id of the document, placed on DOCTYPE declaration
    /// @param system_id const char *, system id of the document, placed on DOCTYPE declaration
    CXmlDoc(const char *name, const char *public_id = 0, const char *system_id = 0);

    /// @brief Destructor
    virtual ~CXmlDoc()
    {
        clear();
    }

    /// @brief Returns node type
    virtual CXmlNodeType type() const
    {
        return DOM_DOCUMENT;
    }

    /// @brief Destroys all nodes in document
    virtual void clear();

    /// @brief Returns the node name.
    virtual const std::string& name() const;

    /// @brief Sets the new name for the node
    /// @param name const std::string&, new node name
    virtual void name(const std::string& name)
    {
    }

    /// @brief Sets new name for node
    /// @param name const char *, new node name
    virtual void name(const char *name)
    {
    }

    /// @brief Returns doctype of document.
    ///
    /// You can use it to add e.g. custom entities.
    /// <pre>
    /// mydoc->doctype().set_entity("myentity", "myreplacement");
    /// </pre>
    CXmlDocType &docType()
    {
        return m_doctype;
    }

    /// @brief Returns doctype of document.
    ///
    /// You can use it to add e.g. custom entities.
    /// <pre>
    /// mydoc->doctype().set_entity("myentity", "myreplacement");
    /// </pre>
    const CXmlDocType &docType() const
    {
        return m_doctype;
    }

    /// @brief Returns pointer to root element of document
    CXmlNode *rootNode();

    /// @brief Returns indentation in save
    int indentSpaces()
    {
        return m_indentSpaces;
    }

    /// @brief Set indentation in save, defaults to 2
    ///
    /// @param i as new indent spaces
    void indentSpaces(int i)
    {
        m_indentSpaces = i;
    }

    /// @brief Loads document from buffer.
    /// @param buffer const char*, source buffer
    virtual void load(const char* buffer);

    /// @brief Loads document from buffer.
    /// @param buffer const CBuffer&, source buffer
    virtual void load(const CBuffer& buffer)
    {
        load(buffer.c_str());
    }

    /// @brief Saves document to buffer.
    /// @param buffer CBuffer&, a buffer to save document
    /// @param indent int, how many indent spaces at start
    virtual void save(CBuffer& buffer, int indent = 0) const;
};
/// @}
}
#endif
