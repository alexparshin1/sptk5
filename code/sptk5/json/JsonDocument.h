/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonDocument.h - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 16 2013                                   ║
║  copyright            © 1999-2019 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __JSON_DOCUMENT_H__
#define __JSON_DOCUMENT_H__

#include "JsonElement.h"
#include "JsonArrayData.h"
#include <istream>
#include <sptk5/Buffer.h>

namespace sptk { 
namespace json {

/// @addtogroup JSON
/// @{

/**
 * JSON document
 */
class SP_EXPORT Document
{
    friend class ObjectData;
    friend class Element;

    Element*        m_root;             ///< Root element of the document
    SharedStrings   m_sharedStrings;    ///< Shared string table for element names
    const Element   m_emptyElement;     ///< Empty element

    /**
     * Parse JSON text, replacing current document content
     * @param json              JSON text
     */
    void parse(const String& json);

public:

    /**
    * Constructor
    * Creates empty JSON document.
    * Use one of the load() methods to populate it, or use add(), remove() methods of the root element to modify it.
    * @param isObject           If true then document root is JSON object. Otherwise, document root is JSON array.
    */
    explicit Document(bool isObject=true);

    /**
     * Copy constructor
     * @param other             Object to copy from
     */
    Document(const Document& other);

    /**
     * Move constructor
     * Creates empty JSON document.
     * Use one of the load() methods to populate it, or use add(), remove() methods of the root element to modify it.
     * @param other             Object to move
     */
    Document(Document&& other) noexcept;

    /**
     * Destructor
     */
    ~Document();

    /**
     * Disable copy assignment
     */
    Document& operator = (const Document&) = delete;

    /**
     * Load document from JSON text, replacing existing document
     * @param json              JSON text
     */
    void load(const String& json);

    /**
     * Load document from JSON text, replacing existing document
     * @param json              JSON text
     */
    void load(const char* json);

    /**
     * Load document from JSON text, replacing existing document
     * @param json              JSON text
     */
    void load(std::istream& json);

    /**
     * Export JSON element (and all children) to stream
     * @param stream            Stream to export JSON
     * @param formatted         If true then JSON text is nicely formatted, but takes more space
     */
    void exportTo(std::ostream& stream, bool formatted=true) const;

    /**
     * Export JSON element (and all children) to buffer
     * @param buffer            Buffer to export JSON
     * @param formatted         If true then JSON text is nicely formatted, but takes more space
     */
    void exportTo(Buffer& buffer, bool formatted=true) const;

    /**
     * Export JSON element (and all children) to XML document
     * @param document          XML document to export JSON
     * @param rootNodeName      XML document root node name
     */
    void exportTo(xml::Document& document, const String& rootNodeName = "data") const;

    /**
     * Get document root element
     */
    Element& root();

    /**
     * Get document root element
     */
    const Element& root() const;

    /**
     * Clear document content
     */
    void clear();

protected:

    /**
     * Return empty const JSON element
     * @return                  Empty const JSON element reference
     */
    const Element& getEmptyElement() const
    {
        return m_emptyElement;
    }

    /**
     * Get shared string matching passed string
     * @param str               String
     * @return                  Shared string
     */
    const std::string* getString(const std::string& str)
    {
        return &m_sharedStrings.shareString(str);
    }

    /**
     * Get shared string matching passed string
     * @param str               String
     * @return                  Shared string
     */
    const std::string* getString(const char* str)
    {
        return &m_sharedStrings.shareString(str);
    }
};

}}

#endif
