#include "XPath.h"
#include <sptk5/RegularExpression.h>
#include <sptk5/xdoc/Attributes.h>
#include <sptk5/xdoc/Document.h>
#include <sptk5/cutils>

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

void NodeSearchAlgorithms::matchNodesThisLevel(Node& thisNode, Node::Vector& nodes,
                                               const vector<XPathElement>& pathElements, int pathPosition,
                                               const String& starPointer, Node::Vector& matchedNodes, bool descendants)
{
    const XPathElement& pathElement = pathElements[size_t(pathPosition)];

    for (auto& node: thisNode)
    {
        if (matchPathElement(&node, pathElement, starPointer))
        {
            matchedNodes.push_back(&node);
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
        Node* anode = matchedNodes[matchedPosition];
        matchedNodes.clear();
        matchedNodes.push_back(anode);
    }

    for (auto* node: matchedNodes)
    {
        matchNode(*node, nodes, pathElements, pathPosition, starPointer);
    }
}

void NodeSearchAlgorithms::scanDescendents(Node& thisNode, Node::Vector& nodes,
                                           const std::vector<XPathElement>& pathElements, int pathPosition,
                                           const String& starPointer)
{
    Node::Vector matchedNodes;
    matchNodesThisLevel(thisNode, nodes, pathElements, pathPosition, starPointer, matchedNodes, true);
}

void NodeSearchAlgorithms::matchNode(Node& thisNode, Node::Vector& nodes,
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
            nodes.push_back(&thisNode);
        }
        return;
    }

    Node::Vector matchedNodes;
    matchNodesThisLevel(thisNode, nodes, pathElements, pathPosition, starPointer, matchedNodes, false);
}

void NodeSearchAlgorithms::select(Node::Vector& nodes, Node& start, String xpath)
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

#if USE_GTEST

static const String testXML1("<AAA><BBB/><CCC/><BBB/><BBB/><DDD><BBB/></DDD><CCC/></AAA>");
static const String testXML2("<AAA><BBB/><CCC/><BBB/><DDD><BBB/></DDD><CCC><DDD><BBB/><BBB/></DDD></CCC></AAA>");
static const String testXML3(
    "<AAA><XXX><DDD><BBB/><BBB/><EEE/><FFF/></DDD></XXX><CCC><DDD><BBB/><BBB/><EEE/><FFF/></DDD></CCC><CCC><BBB><BBB><BBB/></BBB></BBB></CCC></AAA>");
static const String testXML4("<AAA><BBB>1</BBB><BBB>2</BBB><BBB>3</BBB><BBB>4</BBB></AAA>");
static const String testXML5(R"(<AAA><BBB>1</BBB><BBB id="002">2</BBB><BBB id="003">3</BBB><BBB>4</BBB></AAA>)");

TEST(SPTK_XDocument, select)
{
    Node::Vector elementSet;
    Document document;

    document.load(DataFormat::XML, testXML1);

    document.select(elementSet, "/AAA");
    EXPECT_EQ(size_t(1), elementSet.size());

    document.select(elementSet, "/AAA/CCC");
    EXPECT_EQ(size_t(2), elementSet.size());

    document.select(elementSet, "/AAA/DDD/BBB");
    EXPECT_EQ(size_t(1), elementSet.size());
}

TEST(SPTK_XDocument, select2)
{
    Node::Vector elementSet;
    Document document;

    document.load(DataFormat::XML, testXML2);

    document.select(elementSet, "//BBB");
    EXPECT_EQ(size_t(5), elementSet.size());

    document.select(elementSet, "//DDD/BBB");
    EXPECT_EQ(size_t(3), elementSet.size());
}

TEST(SPTK_XDocument, select3)
{
    Node::Vector elementSet;
    Document document;

    document.load(DataFormat::XML, testXML3);

    Buffer buff;
    document.exportTo(DataFormat::XML, buff, false);

    document.select(elementSet, "/AAA/CCC/DDD/*");
    EXPECT_EQ(size_t(4), elementSet.size());

    document.select(elementSet, "//*");
    EXPECT_EQ(size_t(17), elementSet.size());
}

TEST(SPTK_XDocument, select4)
{
    Node::Vector elementSet;
    Document document;

    document.load(DataFormat::XML, testXML4);

    document.select(elementSet, "/AAA/BBB[1]");
    EXPECT_EQ(size_t(1), elementSet.size());
    EXPECT_STREQ("1", elementSet[0]->getString().c_str());

    document.select(elementSet, "/AAA/BBB[last()]");
    EXPECT_EQ(size_t(1), elementSet.size());
    EXPECT_STREQ("4", elementSet[0]->getString().c_str());
}

TEST(SPTK_XDocument, select5)
{
    Node::Vector elementSet;
    Document document;

    document.load(DataFormat::XML, testXML5);

    document.select(elementSet, "//BBB[@id=002]");
    EXPECT_EQ(size_t(1), elementSet.size());
    EXPECT_STREQ("2", elementSet[0]->getString().c_str());

    document.select(elementSet, "//BBB[@id=003]");
    EXPECT_EQ(size_t(1), elementSet.size());
    EXPECT_STREQ("3", elementSet[0]->getString().c_str());
}

#endif
