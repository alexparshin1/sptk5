/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       DocType.h - description                                ║
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

#ifndef __SPTK_XML_DOC_TYPE_H__
#define __SPTK_XML_DOC_TYPE_H__

#include <sptk5/xml/Entities.h>

#include <string>
#include <map>

namespace sptk {

namespace xml {

/**
 * @addtogroup XML
 * @{
 */

/**
 * @brief XML document type.
 *
 * Represents tag <DOCTYPE ...> in XML document.
 * It can return a map of all entities().
 * Provides the name(), public_id() and system_id() functions.
 */
class DocType
{
    friend class Document;

    /**
     * The buffer used to return replacement literals
     */
    char m_replacementBuffer[16];

    /**
     * Encode buffers
     */
    Buffer m_encodeBuffers[2];

public:

    /**
     * @brief Default constructor
     */
    DocType()
    {
    }

    /**
     * @brief Constructor
     */
    DocType(const char* name, const char* public_id = 0, const char* system_id = 0);

    /**
     * @brief Returns the name of the document type as specified in the <!DOCTYPE name> tag
     */
    const std::string& name() const
    {
        return m_name;
    }

    /**
     * @brief Returns the public identifier of the external DTD subset
     *
     * Returns empty string if there is no public identifier
     */
    const std::string& publicID() const
    {
        return m_public_id;
    }

    /**
     * @brief Returns the system identifier of the external DTD subset.
     *
     * Returns empty string if there is no system identifier
     */
    const std::string& systemID() const
    {
        return m_system_id;
    }

    /**
     * @brief Returns a map of all entities described in the DTD
     *
     * NOTE: Map doesn't hold default entities.
     */
    Entities& entities()
    {
        return m_entities;
    }

    /**
     * @brief Returns a map of all entities described in the DTD
     *
     * NOTE: Map doesn't hold default entities.
     */
    const Entities& entities() const
    {
        return m_entities;
    }

    /**
     * @brief Encodes string to XML representation.
     *
     * Converts "<test>" to "&lt;test&gt;"
     * @returns true, any entities replaced.
     * @param str const char *, string to convert
     * @param ret CBuffer&, converted text is stored here
     */
    bool encodeEntities(const char* str, Buffer& ret);

    /**
     * @brief Decodes entities in string to their actual values.
     *
     * Converts "&lt;test&gt;" to "<test>"
     * @param str const char*, text to convert
     * @param sz uint32_t, text length
     * @param ret CBuffer&, converted text is stored here
     */
    void decodeEntities(const char* str, uint32_t sz, Buffer& ret);

    /**
     * @brief Searches for entity with given name
     *
     * @returns true, if attribute is found.
     * @param name const char *, entity to search
     */
    bool hasEntity(const char* name);

    /**
     * @brief Removes named entity from entity map
     * @param name entity to remove
     */
    void removeEntity(const char* name)
    {
        m_entities.removeEntity(name);
    }

    /**
     * @brief Returnes replacement value for named entity.
     *
     * If entity is not found, empty string is returned.
     * @param name const char *, entity name
     * @param replacementLength uint32_t&, the length of the replacement 
     */
    const char* getReplacement(const char* name, uint32_t& replacementLength);

    /**
     * @brief Adds an entity to the map
     *
     * If entity named 'name' exists already in map,
     * its value is replaced with 'replacement'
     * @param name entity to add/change
     * @param replacement value that represents entity
     */
    void setEntity(const char* name, const char* replacement)
    {
        m_entities.setEntity(name, replacement);
    }

private:
    /**
     * List of entities
     */
    Entities m_entities;


    /**
     * Document type name
     */
    std::string m_name;

    /**
     * Public ID
     */
    std::string m_public_id;

    /**
     * System ID
     */
    std::string m_system_id;

};

/**
 * @}
 */
}
}
#endif
