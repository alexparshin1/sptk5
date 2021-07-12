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

#include <sptk5/xdoc/Node.h>

namespace sptk::xdoc {

/**
 * XPath Axis enum
 */
enum class XPathAxis
    : uint8_t
{
    CHILD,      ///< Child axis
    DESCENDANT, ///< Descendant axis
    PARENT      ///< Parent Axis
};

/**
 * Parsed element of XPath
 */
class SP_EXPORT XPathElement
{
public:
    String elementName;                   ///< Node name, or '*'
    String criteria;                      ///< Criteria
    XPathAxis axis {XPathAxis::CHILD};       ///< Axis
    String attributeName;                 ///< Attribute name (optional)
    String attributeValue;                ///< Attribute value (optional)
    bool attributeValueDefined {false}; ///< true if attribute value was defined
    int nodePosition {0};              ///< 0 (not required), -1 (last), or node position
};

/**
 * Algorithms for searching nodes
 */
class SP_EXPORT NodeSearchAlgorithms
{
public:
    /**
     * Scan descendents nodes
     */
    static void scanDescendents(const Node* thisNode, Node::Nodes& nodes, const std::vector<XPathElement>& pathElements,
                                int pathPosition,
                                const String& starPointer);

    /**
     * Match nodes
     */
    static void matchNode(Node* thisNode, Node::Nodes& nodes, const std::vector<XPathElement>& pathElements,
                          int pathPosition,
                          const String& starPointer);

    /**
     * Match nodes only this level
     */
    static void matchNodesThisLevel(const Node* thisNode, Node::Nodes& nodes,
                                    const std::vector<XPathElement>& pathElements, int pathPosition,
                                    const String& starPointer, Node::Nodes& matchedNodes, bool descendants);

    /**
     * Match path element
     */
    static bool matchPathElement(const Node* thisNode, const XPathElement& pathElement,
                                 const String& starPointer);

    static bool matchPathElementAttribute(const Node* thisNode, const XPathElement& pathElement,
                                          const String& starPointer);
};

}
