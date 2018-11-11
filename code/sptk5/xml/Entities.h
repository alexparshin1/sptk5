/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Entities.h - description                               ║
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

#ifndef __SPTK_XML_ENTITIES_H__
#define __SPTK_XML_ENTITIES_H__

#include <string>
#include <map>

namespace sptk {
namespace xml {

/**
 * @addtogroup XML
 * @{
 */

/**
 * @brief XML entities
 *
 * Maps an XML entity string to a presentation string.
 */
class Entities : public std::map<std::string, std::string>
{
public:

    /**
     * @brief Constructor
     */
    Entities() {}

    /**
     * @brief Removes named entity
     * @param name const char *, entity name to remove
     */
    void removeEntity(const char* name)
    {
        erase(name);
    }

    /**
     * @brief Adds entity to map
     *
     * If entity named 'name' exists already in map, its value is replaced with 'replacement'
     * @param name const char *, entity to add/change
     * @param replacement const char *, value that represents entity
     */
    void setEntity(const char* name, const char* replacement)
    {
        (*this)[name] = replacement;
    }
};
/**
 * @}
 */
}
}
#endif
