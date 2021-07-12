#include "XPath.h"
#include <sptk5/RegularExpression.h>
#include <sptk5/xdoc/Attributes.h>

using namespace std;
using namespace sptk;
using namespace xdoc;

static void makeCriteria(XPathElement& pathElement)
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

static void parsePathElement(const string& pathElementStr, XPathElement& pathElement)
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
    auto pathElementOption = option;

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

bool NodeSearchAlgorithms::matchPathElementAttribute(const Node* thisNode, const XPathElement& pathElement,
                                                     const String& starPointer)
{
    const Attributes& attributes = thisNode->attributes();
    bool attributeMatch = false;
    if (pathElement.attributeValueDefined)
    {
        if (pathElement.attributeValue == starPointer)
        {
            attributeMatch = attributes.has(pathElement.attributeName);
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
            attributeMatch = thisNode->hasAttribute(pathElement.attributeName.c_str());
        }
    }
    return attributeMatch;
}

bool NodeSearchAlgorithms::matchPathElement(const Node* thisNode, const XPathElement& pathElement,
                                            const String& starPointer)
{
    if (!pathElement.elementName.empty() && pathElement.elementName != starPointer &&
        thisNode->name() != pathElement.elementName)
    {
        return false;
    }

    // Node criteria is attribute
    if (!pathElement.attributeName.empty())
    {
        return matchPathElementAttribute(thisNode, pathElement, starPointer);
    }
    return true;
}

void NodeSearchAlgorithms::matchNodesThisLevel(const Node* thisNode, Node::Nodes& nodes,
                                               const vector<XPathElement>& pathElements, int pathPosition,
                                               const String& starPointer, Node::Nodes& matchedNodes, bool descendants)
{
    const XPathElement& pathElement = pathElements[size_t(pathPosition)];

    for (const auto& node: *thisNode)
    {
        if (matchPathElement(&node, pathElement, starPointer))
        {
            matchedNodes.push_back(node);
        }
        if (descendants)
        {
            scanDescendents(&node, nodes, pathElements, pathPosition, starPointer);
        }
        else
        {
            if (pathElement.axis == XPathAxis::DESCENDANT)
            {
                scanDescendents(&node, nodes, pathElements, pathPosition, starPointer);
            }
        }
    }

    if (matchedNodes.empty())
    {
        return;
    }

    if (pathElement.nodePosition != 0)
    {
        int matchedPosition = 0;
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
        Node& anode = matchedNodes[matchedPosition];
        matchedNodes.clear();
        matchedNodes.push_back(anode);
    }

    for (auto& node: matchedNodes)
    {
        matchNode(&node, nodes, pathElements, pathPosition, starPointer);
    }
}

void NodeSearchAlgorithms::scanDescendents(const Node* thisNode, Node::Nodes& nodes,
                                           const std::vector<XPathElement>& pathElements, int pathPosition,
                                           const String& starPointer)
{
    Node::Nodes matchedNodes;
    matchNodesThisLevel(thisNode, nodes, pathElements, pathPosition, starPointer, matchedNodes, true);
}

void NodeSearchAlgorithms::matchNode(Node* thisNode, Node::Nodes& nodes, const vector<XPathElement>& pathElements,
                                     int pathPosition,
                                     const String& starPointer)
{
    ++pathPosition;
    if (pathPosition == (int) pathElements.size())
    {
        if (const XPathElement& pathElement = pathElements[size_t(pathPosition - 1)];
            !pathElement.elementName.empty())
        {
            nodes.push_back(*thisNode);
        }
        else if (!pathElement.attributeName.empty())
        {
            /*
            Attribute* attributeNode = thisNode->attributes().getAttributeNode(pathElement.attributeName);
            if (attributeNode != nullptr)
            {
                nodes.insert(nodes.end(), dynamic_cast<Node*>(attributeNode));
            }
             */
        }
        return;
    }

    Node::Nodes matchedNodes;
    matchNodesThisLevel(thisNode, nodes, pathElements, pathPosition, starPointer, matchedNodes, false);
}

void select(Node::Nodes& nodes, xdoc::Node& start, String xpath)
{
    nodes.clear();

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
    NodeSearchAlgorithms::matchNode(&start, nodes, pathElements, -1, starPointer);
}
