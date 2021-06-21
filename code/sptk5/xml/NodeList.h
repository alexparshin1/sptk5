/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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
#include <vector>
#include <algorithm>

namespace sptk::xml {

/**
 * @addtogroup XML
 * @{
 */

class Node;

/**
 * The vector of xml::Node *
 */
using NodeVector = std::vector<Node*>;

/**
 * XML node list
 *
 * The xml::NodeList interface provides the an ordered collection of nodes,
 * The items in the NodeList are accessible via an integral index, starting from 0.
 */
class SP_EXPORT NodeList : public NodeVector
{
public:
    /**
     * Constructor
     */
    NodeList() noexcept
    {}

    NodeList(const NodeList&) = delete;
    NodeList(NodeList&&) noexcept = default;
    NodeList& operator = (const NodeList&) = delete;
    NodeList& operator = (NodeList&&) noexcept = default;

    /**
     * Destructor
     */
    ~NodeList() noexcept
    {
        clear();
    }

    /**
     * Clears the list of XML nodes and releases all the allocated memory
     */
    void clear();

    /**
     * Finds the first node in the list with the matching name
     * @param nodeName const std::string&, a node name
     * @returns node iterator, or end()
     */
    iterator findFirst(const String& nodeName);

    /**
     * Finds the first node node in the list with the matching name
     * @param nodeName const std::string&, a node name
     * @returns node iterator, or end()
     */
    const_iterator findFirst(const String& nodeName) const;
};
/**
 * @}
 */
}
