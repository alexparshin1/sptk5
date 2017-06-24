/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       JsonDocument.h - description                           ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 16 2013                                   ║
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

#ifndef __JSON_DOCUMENT_H__
#define __JSON_DOCUMENT_H__

#include "JsonElement.h"
#include "JsonArrayData.h"
#include <istream>
#include <sptk5/Buffer.h>

namespace sptk { namespace json {

/// @addtogroup JSON
/// @{

/**
 * JSON document
 */
class Document
{
    /**
     * Root element of the document
     */
    Element*    m_root;

    /**
     * Parse JSON text, replacing current document content
     * @param json const std::string&, JSON text
     * @throw Exception if there is a problem parsing JSON document
     */
    void parse(const std::string& json) throw(Exception);

    /**
     * Disable copy constructor
     */
    Document(const Document&) : m_root(NULL) {}

    /**
     * Disable copy assignment
     */
    Document& operator = (const Document&) { return *this; }

public:
    /**
     * Constructor
     * Creates empty JSON document.
     * Use one of the load() methods to populate it, or use add(), remove() methods of the root element to modify it.
     * @param isObject bool, If true then document root is JSON object. Otherwise, document root is JSON array.
     */
    Document(bool isObject=true);

    /**
     * Move constructor
     * Creates empty JSON document.
     * Use one of the load() methods to populate it, or use add(), remove() methods of the root element to modify it.
     * @param isObject bool, If true then document root is JSON object. Otherwise, document root is JSON array.
     */
    Document(Document&& other);

    /**
     * Destructor
     */
    ~Document();

    /**
     * Load document from JSON text, replacing existing document
     * @param json const std::string&, JSON text
     * @throw Exception if there is a problem parsing JSON document
     */
    void load(const std::string& json) throw(Exception);

    /**
     * Load document from JSON text, replacing existing document
     * @param json const char*, JSON text
     * @throw Exception if there is a problem parsing JSON document
     */
    void load(const char* json) throw(Exception);

    /**
     * Load document from JSON text, replacing existing document
     * @param json std::istream&, JSON text
     * @throw Exception if there is a problem parsing JSON document
     */
    void load(std::istream& json) throw(Exception);

    /**
     * Export JSON element (and all children) to stream
     * @param stream std::ostream&, Stream to export JSON
     * @param formatted bool, If true then JSON text is nicely formatted, but takes more space
     */
    void exportTo(std::ostream& stream, bool formatted=true);

    /**
     * Export JSON element (and all children) to stream
     * @param buffer sptk::Buffer&, Buffer to export JSON
     * @param formatted bool, If true then JSON text is nicely formatted, but takes more space
     */
    void exportTo(Buffer& buffer, bool formatted=true);

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
};

}}

#endif
