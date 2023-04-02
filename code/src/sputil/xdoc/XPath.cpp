/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#include "XPath.h"
#include <sptk5/RegularExpression.h>

using namespace std;
using namespace sptk;
using namespace xdoc;

namespace {
void makeCriteria(XPathElement& pathElement)
{
    static const RegularExpression matchAttribute(R"(@(?<attribute>[\w\-_:]+)=['"]?(?<value>.*)['"]?)");

    const String& criteria = pathElement.criteria;

    if (auto matches = matchAttribute.m(pathElement.criteria);
        matches)
    {
        pathElement.attributeName = matches["attribute"].value;
        pathElement.attributeValue = matches["value"].value;
        pathElement.attributeValueDefined = true;
        return;
    }

    if (!criteria.empty())
    {
        int& nodePosition = pathElement.nodePosition;
        nodePosition = string2int(pathElement.criteria);
        if (nodePosition == 0 && criteria == "last()")
        {
            nodePosition = -1;
        }
    }
}

void parsePathElement(const string& pathElementStr, XPathElement& pathElement)
{
    static const RegularExpression matchPathElement(
        R"((?<type>(descendant|parent)::)?(?<element>([\w\-_:]+|\*))(?<option>\[.*\])?)");

    auto matches = matchPathElement.m(pathElementStr);
    if (!matches)
    {
        throw Exception("Invalid XML path element: " + pathElementStr);
    }

    auto pathElementType = matches["type"].value;
    auto pathElementName = matches["element"].value;

    // Compensating bug in PCRE
    auto option = matches["option"].value;
    if (!option.empty())
    {
        option = option.substr(1, option.length() - 2);
    }
    const auto& pathElementOption = option;

    pathElement.elementName = "";
    pathElement.attributeName = "";
    pathElement.axis = XPathAxis::CHILD;

    if (!pathElementOption.empty())
    {
        pathElement.criteria = pathElementOption;
        makeCriteria(pathElement);
    }

    if (pathElementType.startsWith("descendant"))
    {
        pathElement.axis = XPathAxis::DESCENDANT;
    }
    else if (pathElementType.startsWith("parent"))
    {
        pathElement.axis = XPathAxis::PARENT;
    }

    if (pathElementName[0] == '@')
    {
        pathElement.attributeName = pathElementName.c_str() + 1;
    }
    else
    {
        pathElement.elementName = pathElementName.c_str();
    }
}
} // namespace

bool NodeSearchAlgorithms::matchPathElementAttribute(const SNode& thisNode, const XPathElement& pathElement,
                                                     const String& starPointer)
{
    const Attributes& attributes = thisNode->attributes();
    bool attributeMatch;
    if (pathElement.attributeValueDefined)
    {
        if (pathElement.attributeValue == starPointer)
        {
            attributeMatch = attributes.have(pathElement.attributeName);
        }
        else
        {
            attributeMatch = attributes.get(pathElement.attributeName) == pathElement.attributeValue;
        }
    }
    else
    {
        if (pathElement.attributeName == starPointer)
        {
            attributeMatch = !attributes.empty();
        }
        else
        {
            attributeMatch = thisNode->attributes().have(pathElement.attributeName.c_str());
        }
    }
    return attributeMatch;
}

bool NodeSearchAlgorithms::matchPathElement(const SNode& thisNode, const XPathElement& pathElement,
                                            const String& starPointer)
{
    if (!pathElement.elementName.empty() && pathElement.elementName != starPointer &&
        thisNode->name() != pathElement.elementName)
    {
        return false;
    }

    // Node criteria is an attribute
    if (!pathElement.attributeName.empty())
    {
        return matchPathElementAttribute(thisNode, pathElement, starPointer);
    }

    return true;
}

void NodeSearchAlgorithms::matchNodesThisLevel(const SNode& thisNode, Node::Vector& nodes,
                                               const vector<XPathElement>& pathElements, int pathPosition,
                                               const String& starPointer, Node::Vector& matchedNodes, bool descendants)
{
    const XPathElement& pathElement = pathElements[size_t(pathPosition)];

    for (const auto& node: thisNode->nodes())
    {
        if (matchPathElement(node, pathElement, starPointer))
        {
            matchedNodes.push_back(node);
        }

        if (descendants)
        {
            scanDescendents(node, nodes, pathElements, pathPosition, starPointer);
        }
        else
        {
            if (pathElement.axis == XPathAxis::DESCENDANT)
            {
                scanDescendents(node, nodes, pathElements, pathPosition, starPointer);
            }
        }
    }

    if (matchedNodes.empty())
    {
        return;
    }

    if (pathElement.nodePosition != 0)
    {
        int matchedPosition;
        if (pathElement.nodePosition < 0)
        {
            matchedPosition = int(matchedNodes.size() + pathElement.nodePosition);
        }
        else
        {
            matchedPosition = pathElement.nodePosition - 1;
        }
        
        if (matchedPosition < 0 || matchedPosition >= (int) matchedNodes.size())
        {
            return;
        }
        auto anode = matchedNodes[matchedPosition];
        matchedNodes.clear();
        matchedNodes.push_back(anode);
    }

    for (const auto& node: matchedNodes)
    {
        matchNode(node, nodes, pathElements, pathPosition, starPointer);
    }
}

void NodeSearchAlgorithms::scanDescendents(const SNode& thisNode, Node::Vector& nodes,
                                           const std::vector<XPathElement>& pathElements, int pathPosition,
                                           const String& starPointer)
{
    Node::Vector matchedNodes;
    matchNodesThisLevel(thisNode, nodes, pathElements, pathPosition, starPointer, matchedNodes, true);
}

void NodeSearchAlgorithms::matchNode(const SNode& thisNode, Node::Vector& nodes,
                                     const vector<XPathElement>& pathElements,
                                     int pathPosition,
                                     const String& starPointer)
{
    ++pathPosition;
    if (pathPosition == (int) pathElements.size())
    {
        if (const XPathElement& pathElement = pathElements[size_t(pathPosition - 1)];
            !pathElement.elementName.empty())
        {
            nodes.push_back(thisNode);
        }
        return;
    }

    Node::Vector matchedNodes;
    matchNodesThisLevel(thisNode, nodes, pathElements, pathPosition, starPointer, matchedNodes, false);
}

void NodeSearchAlgorithms::select(Node::Vector& nodes, const SNode& start, String xpath)
{
    if (!xpath.startsWith("/"))
    {
        xpath = "//" + xpath;
    }

    xpath = xpath.replace("\\/\\/", "/descendant::");

    const char* ptr = xpath[0] == '/' ? xpath.c_str() + 1 : xpath.c_str();

    Strings pathElementStrs(ptr, "/");
    vector<XPathElement> pathElements(pathElementStrs.size());
    for (size_t i = 0; i < pathElements.size(); ++i)
    {
        parsePathElement(pathElementStrs[i], pathElements[i]);
    }

    const String starPointer("*");
    NodeSearchAlgorithms::matchNode(start, nodes, pathElements, -1, starPointer);
}
