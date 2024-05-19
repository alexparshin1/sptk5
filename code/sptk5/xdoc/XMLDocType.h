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

#pragma once

#include <sptk5/Buffer.h>
#include <sptk5/xdoc/XMLEntities.h>

#include <map>
#include <string>

namespace sptk::xdoc {

/**
 * @addtogroup XML
 * @{
 */

/**
 * XML document type.
 *
 * Represents tag <DOCTYPE ...> in XML document.
 * It can return a map of all entities().
 * Provides the name(), public_id() and system_id() functions.
 */
class SP_EXPORT XMLDocType
{
    friend class ImportXML;

public:
    /**
     * Default constructor
     */
    XMLDocType() = default;

    /**
     * Constructor
     */
    explicit XMLDocType(const char* name, const char* public_id = nullptr, const char* system_id = nullptr);

    /**
     * Returns the name of the document type as specified in the <!DOCTYPE name> tag
     */
    const String& name() const
    {
        return m_name;
    }

    /**
     * Returns the public identifier of the external DTD subset
     *
     * Returns empty string if there is no public identifier
     */
    const String& publicID() const
    {
        return m_public_id;
    }

    /**
     * Returns the system identifier of the external DTD subset.
     *
     * Returns empty string if there is no system identifier
     */
    const String& systemID() const
    {
        return m_system_id;
    }

    /**
     * Returns a map of all entities described in the DTD
     *
     * NOTE: Map doesn't hold default entities.
     */
    Entities& entities()
    {
        return m_entities;
    }

    /**
     * Returns a map of all entities described in the DTD
     *
     * NOTE: Map doesn't hold default entities.
     */
    const Entities& entities() const
    {
        return m_entities;
    }

    /**
     * Encodes string to XML representation.
     *
     * Converts "<test>" to "&lt;test&gt;"
     * @returns true, any entities replaced.
     * @param str               String to convert
     * @param ret               Converted text is stored here
     */
    bool encodeEntities(const char* str, Buffer& ret);

    /**
     * Decodes entities in string to their actual values.
     *
     * Converts "&lt;test&gt;" to "<test>"
     * @param str               Text to convert
     * @param size                Text length
     * @param ret               Converted text is stored here
     */
    void decodeEntities(const char* str, size_t size, Buffer& ret);

    /**
     * Removes named entity from entity map
     * @param name              Entity to remove
     */
    void removeEntity(const char* name)
    {
        m_entities.removeEntity(name);
    }

    /**
     * Returnes replacement value for named entity.
     *
     * If entity is not found, empty string is returned.
     * @param name              Entity name
     * @param replacementLength Length of the replacement
     */
    const char* getReplacement(const char* name, uint32_t& replacementLength);

    /**
     * Adds an entity to the map
     *
     * If entity named 'name' exists already in map,
     * its value is replaced with 'replacement'
     * @param name              Entity to add/change
     * @param replacement       Value that represents entity
     */
    void setEntity(const String& name, const String& replacement)
    {
        m_entities.setEntity(name, replacement);
    }

private:
    std::array<char, 16> m_replacementBuffer {}; ///< The buffer used to return replacement literals
    std::array<Buffer, 2> m_encodeBuffers;       ///< Encode buffers
    Entities m_entities;                         ///< List of entities
    String m_name;                               ///< Document type name
    String m_public_id;                          ///< Public ID
    String m_system_id;                          ///< System ID

    char* appendDecodedEntity(Buffer& ret, const char* ent_start, char* ent_end);
};

/**
 * @}
 */
} // namespace sptk::xdoc
