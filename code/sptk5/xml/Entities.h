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

#pragma once

#include <string>
#include <map>
#include <sptk5/RegularExpression.h>

namespace sptk {
namespace xml {

/**
 * @addtogroup XML
 * @{
 */

/**
 * XML document type entity
 */
class Entity
{
public:
    enum Type { SYSTEM, PUBLIC };
    String  name;
    Type    type;
    String  id;
    String  resource;
    void parse(const String& entityTag);
};

/**
 * XML entities
 *
 * Maps an XML entity string to a presentation string.
 */
class Entities : public std::map<String, String>
{
public:

    /**
     * Constructor
     */
    Entities() = default;

    /**
     * Removes named entity
     * @param name              entity name to remove
     */
    void removeEntity(const String& name)
    {
        erase(name);
    }

    /**
     * Adds entity to map
     *
     * If entity named 'name' exists already in map, its value is replaced with 'replacement'
     * @param name              entity to add/change
     * @param replacement       value that represents entity
     */
    void setEntity(const String& name, const String& replacement)
    {
        (*this)[name] = replacement;
    }
};
/**
 * @}
 */
}
}
