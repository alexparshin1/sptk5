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

#include <sptk5/Exception.h>
#include <sptk5/cxml>

#include <sptk5/json/JsonDocument.h>
#include <sptk5/cutils>

using namespace std;
using namespace sptk;
using namespace sptk::xml;

/// An empty string to use as a stub for value()
static const String emptyString;
static const String indentsString(1024, ' ');

/// An empty nodes set to emulate a set of stub iterators
static NodeList emptyNodes;

Node::iterator NodeIterators::begin()
{
    return emptyNodes.end();
}

Node::const_iterator NodeIterators::begin() const
{
    return emptyNodes.end();
}

Node::iterator NodeIterators::end()
{
    return emptyNodes.end();
}

Node::const_iterator NodeIterators::end() const
{
    return emptyNodes.end();
}

//──────────────────────────────────────────────────────────────────────────────

void NodeBase::setParent(Node* p, bool minimal)
{
    if (m_parent == p)
        return;

    if (!minimal && m_parent != nullptr)
        m_parent->unlink(p);

    m_parent = p;

    if (!minimal && m_parent != nullptr)
        m_parent->push_back((Node*)this);
}

//──────────────────────────────────────────────────────────────────────────────

const String& Node::value() const
{
    return emptyString;
}

static void makeCriteria(Document* document, XPathElement& pathElement, size_t pos)
{
    string& criteria = pathElement.criteria;

    if (!criteria.empty()) {
        int& nodePosition = pathElement.nodePosition;
        nodePosition = string2int(pathElement.criteria);
        if (nodePosition == 0 && criteria == "last()")
            nodePosition = -1;

        if (nodePosition == 0 && criteria[0] == '@') {
            pos = criteria.find('=');
            if (pos == STRING_NPOS)
                pathElement.attributeName = criteria.c_str() + 1;
            else {
                pathElement.attributeName = criteria.substr(1, pos - 1);
                if (criteria[pos + 1] == '\'' || criteria[pos + 1] == '"')
                    pathElement.attributeValue = criteria.substr(pos + 2, criteria.length() - (pos + 3));
                else
                    pathElement.attributeValue = criteria.substr(pos + 1, criteria.length() - (pos + 1));
                pathElement.attributeValueDefined = true;
            }
        }
    }
}

static void parsePathElement(Document* document, const string& pathElementStr, XPathElement& pathElement)
{
    pathElement.elementName = "";
    pathElement.attributeName = "";
    pathElement.axis = XPA_CHILD;
    size_t backBracketPosition = pathElementStr.rfind(']');
    string pathElementName;
    if (backBracketPosition == STRING_NPOS) {
        pathElementName = pathElementStr;
        pathElement.criteria.clear();
    } else {
        size_t bracketPosition = pathElementStr.find('[');
        if (bracketPosition == STRING_NPOS || backBracketPosition < bracketPosition) {
            pathElementName = pathElementStr;
            pathElement.criteria.clear();
        } else {
            pathElementName = pathElementStr.substr(0, bracketPosition);
            pathElement.criteria = pathElementStr.substr(bracketPosition + 1,
                                                         backBracketPosition - bracketPosition - 1);
        }
    }

    size_t pos = pathElementName.find("::");
    if (pos != STRING_NPOS) {
        if (pos == 10 && pathElementName.compare(0, 12, "descendant::") == 0)
            pathElement.axis = XPA_DESCENDANT;
        else if (pos == 6 && pathElementName.compare(0, 8, "parent::") == 0)
            pathElement.axis = XPA_DESCENDANT;
        pathElementName.erase(0, pos + 2);
    }

    if (pathElementName[0] == '@')
        pathElement.attributeName = pathElementName.c_str() + 1;
    else
        pathElement.elementName = pathElementName.c_str();

    makeCriteria(document, pathElement, pos);
}

bool NodeSearchAlgorithms::matchPathElement(Node* thisNode, const XPathElement& pathElement, const String& starPointer, bool&)
{
    if (!pathElement.elementName.empty() && pathElement.elementName != starPointer &&
        !thisNode->nameIs(pathElement.elementName))
        return false;

    // Node criteria is attribute
    if (!pathElement.attributeName.empty() && thisNode->type() == Node::DOM_ELEMENT) {
        const Attributes& attributes = thisNode->attributes();
        bool attributeMatch = false;
        if (pathElement.attributeValueDefined) {
            if (pathElement.attributeName == starPointer) {
                for (auto a: attributes) {
                    if (a->name() == pathElement.attributeValue) {
                        attributeMatch = true;
                        break;
                    }
                }
            } else
                attributeMatch =
                        attributes.getAttribute(pathElement.attributeName).asString() == pathElement.attributeValue;
        } else {
            if (pathElement.attributeName == starPointer)
                attributeMatch = thisNode->hasAttributes();
            else
                attributeMatch = thisNode->hasAttribute(pathElement.attributeName.c_str());
        }
        return attributeMatch;
    }
    return true;
}

void NodeSearchAlgorithms::matchNodesThisLevel(const Node* thisNode, NodeVector& nodes, const vector<XPathElement>& pathElements, int pathPosition,
                                               const String& starPointer, NodeVector& matchedNodes, bool descendants)
{
    const XPathElement& pathElement = pathElements[size_t(pathPosition)];

    for (auto* node: *thisNode) {
        bool nameMatches;
        if (matchPathElement(node, pathElement, starPointer, nameMatches)) {
            matchedNodes.push_back(node);
        }
        if (descendants) {
            if ((node->type() & (Node::DOM_DOCUMENT | Node::DOM_ELEMENT)) != 0)
                scanDescendents(node, nodes, pathElements, pathPosition, starPointer);
        } else {
            if (pathElement.axis == XPA_DESCENDANT)
                scanDescendents(node, nodes, pathElements, pathPosition, starPointer);
        }
    }

    if (matchedNodes.empty())
        return;

    if (pathElement.nodePosition != 0) {
        int matchedPosition;
        if (pathElement.nodePosition < 0)
            matchedPosition = int(matchedNodes.size() + pathElement.nodePosition);
        else
            matchedPosition = pathElement.nodePosition - 1;
        if (matchedPosition < 0 || matchedPosition >= (int) matchedNodes.size())
            return;
        Node* anode = matchedNodes[matchedPosition];
        matchedNodes.clear();
        matchedNodes.push_back(anode);
    }

    for (auto* node: matchedNodes)
        matchNode(node, nodes, pathElements, pathPosition, starPointer);
}

void NodeSearchAlgorithms::scanDescendents(const Node* thisNode, NodeVector& nodes, const std::vector<XPathElement>& pathElements, int pathPosition,
                                           const String& starPointer)
{
    NodeVector matchedNodes;
    matchNodesThisLevel(thisNode, nodes, pathElements, pathPosition, starPointer, matchedNodes, true);
}

void NodeSearchAlgorithms::matchNode(Node* thisNode, NodeVector& nodes, const vector<XPathElement>& pathElements, int pathPosition,
                                     const String& starPointer)
{
    ++pathPosition;
    if (pathPosition == (int) pathElements.size()) {
        const XPathElement& pathElement = pathElements[size_t(pathPosition - 1)];
        if (!pathElement.elementName.empty())
            nodes.insert(nodes.end(), thisNode);
        else if (!pathElement.attributeName.empty()) {
            Attribute* attributeNode = thisNode->attributes().getAttributeNode(pathElement.attributeName);
            if (attributeNode != nullptr)
                nodes.insert(nodes.end(), dynamic_cast<Node*>(attributeNode));
        }
        return;
    }

    NodeVector matchedNodes;
    matchNodesThisLevel(thisNode, nodes, pathElements, pathPosition, starPointer, matchedNodes, false);
}

void Node::select(NodeVector& nodes, String xpath)
{
    const char* ptr;
    nodes.clear();

    if (!xpath.startsWith("/"))
        xpath = "//" + xpath;

    xpath = xpath.replace("\\/\\/", "/descendant::");

    if (xpath[0] == '/')
        ptr = xpath.c_str() + 1;
    else
        ptr = xpath.c_str();

    Strings pathElementStrs(ptr, "/");
    vector<XPathElement> pathElements(pathElementStrs.size());
    for (size_t i = 0; i < pathElements.size(); ++i)
        parsePathElement(document(), pathElementStrs[i], pathElements[i]);

    const String starPointer("*");
    NodeSearchAlgorithms::matchNode(this, nodes, pathElements, -1, starPointer);
}

void Node::copy(const Node& node)
{
    name(node.name());
    value(node.value());
    if (isElement() && node.isElement()) {
        attributes() = node.attributes();
    }

    for (const auto* childNode: node) {
        Node* element;
        switch (childNode->type()) {
            case DOM_ELEMENT:
                element = new Element(this, "");
                element->copy(*childNode);
                break;
            case DOM_PI:
                new PI(*this, childNode->name());
                break;
            case DOM_TEXT:
                new Text(*this, childNode->value());
                break;
            case DOM_CDATA_SECTION:
                new CDataSection(*this, childNode->value());
                break;
            case DOM_COMMENT:
                new Comment(*this, childNode->value());
                break;
            default:
                break;
        }
    }
}

String Node::text() const
{
    String ret;

    if ((type() & (DOM_TEXT | DOM_CDATA_SECTION)) != 0)
        ret += value();
    else {
        for (const auto* np: *this) {
            if ((np->type() & (DOM_TEXT | DOM_CDATA_SECTION)) != 0)
                ret += np->value();
        }
    }

    return ret;
}

void Node::text(const String& txt)
{
    clearChildren();
    new Text(*this, txt);
}

void Node::saveElement(const String& nodeName, Buffer& buffer, int indent) const
{
    buffer.append('<');
    if (type() == DOM_PI) buffer.append('?');
    buffer.append(nodeName);
    if (!this->attributes().empty()) {
        // Output attributes
        saveAttributes(buffer);
    }
    if (!empty()) {
        bool only_cdata;
        const Node* nd = *begin();
        if (size() == 1 && nd->type() == DOM_TEXT) {
            only_cdata = true;
            buffer.append('>');
        } else {
            only_cdata = false;
            buffer.append('>');
            if (indent) buffer.append('\n');
        }
        appendSubNodes(buffer, indent, only_cdata);
        appendClosingTag(buffer, indent, only_cdata);
    } else {
        //LEAF
        if (type() == DOM_PI)
            buffer.append(" ?>", 3);
        else
            buffer.append("/>", 2);
        if (indent) buffer.append('\n');
    }
}

void Node::appendSubNodes(Buffer& buffer, int indent, bool only_cdata) const
{
    for (const auto* np: *this) {
        if (only_cdata)
            np->save(buffer, -1);
        else {
            np->save(buffer, indent + document()->indentSpaces());
            if (indent && buffer.data()[buffer.bytes() - 1] != '\n')
                buffer.append('\n');
        }
    }
}

void Node::appendClosingTag(Buffer& buffer, int indent, bool only_cdata) const
{// output indendation spaces
    if (!only_cdata && indent > 0)
        buffer.append(indentsString.c_str(), size_t(indent));

    // output closing tag
    buffer.append("</", 2);
    buffer.append(name());
    buffer.append('>');
    if (indent) buffer.append('\n');
}

void Node::saveAttributes(Buffer& buffer) const
{
    Buffer real_id;
    Buffer real_val;
    for (const auto* attributeNode: attributes()) {
        real_id.bytes(0);
        real_val.bytes(0);
        if (!document()->docType().encodeEntities(attributeNode->name().c_str(), real_id))
            real_id = attributeNode->name();
        if (!document()->docType().encodeEntities(attributeNode->value().c_str(), real_val))
            real_val = attributeNode->value();

        buffer.append(' ');
        buffer.append(real_id);
        buffer.append("=\"");
        buffer.append(real_val);
        buffer.append("\"");
    }
}

void Node::save(Buffer& buffer, int indent) const
{
    // output indendation spaces
    if (indent > 0)
        buffer.append(indentsString.c_str(), size_t(indent));

    const string& nodeName = name();

    // depending on the nodetype, do output
    switch (type()) {
        case DOM_TEXT:
            if (value().substr(0, 9) == "<![CDATA[" && value().substr(value().length() - 3) == "]]>")
                buffer.append(value());
            else
                document()->docType().encodeEntities(value().c_str(), buffer);
            break;

        case DOM_CDATA_SECTION:
            // output all subnodes
            buffer.append("<![CDATA[" + value() + "]]>\n");
            break;

        case DOM_COMMENT:
            // output all subnodes
            buffer.append("<!-- " + value() + " -->\n");
            break;

        case DOM_PI:
        case DOM_ELEMENT:
            saveElement(nodeName, buffer, indent);
            break;

        default: // unknown nodetype
            break;
    }
}

void Node::save(json::Element& json, string& text) const
{
    const string& nodeName = name();

    if (isText()) {
        text.append(this->text());
        return;
    }

    if (isPI()) {
        json.set(name(), value());
        return;
    }

    auto* object = json.add_object(nodeName);
    if (isElement())
        saveAttributes(object);

    switch (type()) {
        case DOM_TEXT:
        case DOM_CDATA_SECTION:
            if (value().substr(0, 9) == "<![CDATA[" && value().substr(value().length() - 3) == "]]>")
                *object = value().substr(9, value().length() - 12);
            else
                *object = value();
            break;

        case DOM_COMMENT:
            // output all subnodes
            object->set("comments", value());
            break;

        case DOM_ELEMENT:
            saveElement(object);
            break;

        default: // unknown nodetype
            break;
    }
}

void Node::saveElement(json::Element* object) const
{
    if (empty()) {
        *object = "";
    } else {
        bool done = false;
        if (size() == 1) {
            for (const auto* np: *this)
                if (np->name() == "null") {
                    done = true;
                }
        }
        if (!done) {
            // output all subnodes
            String nodeText;
            for (const auto* np: *this)
                np->save(*object, nodeText);
            if (object->is(json::JDT_OBJECT) && object->size() == 0) {
                if (Document::isNumber(nodeText)) {
                    double value = string2double(nodeText);
                    *object = value;
                } else
                    *object = nodeText;
            }
        }
    }
}

void Node::saveAttributes(json::Element* object) const
{
    const Attributes& attributes = this->attributes();
    if (!attributes.empty()) {
        auto* attrs = object->add_object("attributes");
        for (const auto* attributeNode: attributes)
            attrs->set(attributeNode->name(), attributeNode->value());
    }
}

void Node::exportTo(json::Element& element) const
{
    string text;
    for (const auto* np: *this)
        np->save(element, text);
}

Node* Node::findFirst(const String& aname, bool recursively) const
{
    for (auto* node: *this) {
        if (node->name() == aname)
            return node;
        if (recursively && !node->empty()) {
            Node* cnode = node->findFirst(aname, true);
            if (cnode != nullptr)
                return cnode;
        }
    }
    return nullptr;
}

Node* Node::findOrCreate(const String& aname, bool recursively)
{
    Node* node = findFirst(aname, recursively);
    if (node != nullptr)
        return node;
    return new Element(*this, aname);
}

String BaseTextNode::nodeName() const
{
    return emptyString;
}

void NamedItem::name(const String& name)
{
    m_name = name;
}

void Element::insert(iterator itor, Node* node)
{
    m_nodes.insert(itor, node);
    node->setParent(this, true);
}

void Element::push_back(Node* node)
{
    m_nodes.insert(m_nodes.end(), node);
    node->setParent(this, true);
}

void Element::unlink(Node* node)
{
    auto itor = find(begin(), end(), node);
    if (itor == end())
        return;
    m_nodes.erase(itor);
}

void Element::remove(Node* node)
{
    auto itor = find(begin(), end(), node);
    if (itor == end())
        return;
    delete *itor;
    m_nodes.erase(itor);
}

void Element::clearChildren()
{
    m_nodes.clear();
}

void Element::clear()
{
    Node::clear();
    m_nodes.clear();
    m_attributes.clear();
}

String Comment::nodeName() const
{
    static const String nodeNameString("#comment");
    return nodeNameString;
}

String Text::nodeName() const
{
    static const String nodeNameString("#text");
    return nodeNameString;
}

String CDataSection::nodeName() const
{
    static const String nodeNameString("#cdata-section");
    return nodeNameString;
}

#if USE_GTEST

static const String testXML1("<AAA><BBB/><CCC/><BBB/><BBB/><DDD><BBB/></DDD><CCC/></AAA>");
static const String testXML2("<AAA><BBB/><CCC/><BBB/><DDD><BBB/></DDD><CCC><DDD><BBB/><BBB/></DDD></CCC></AAA>");
static const String testXML3("<AAA><XXX><DDD><BBB/><BBB/><EEE/><FFF/></DDD></XXX><CCC><DDD><BBB/><BBB/><EEE/><FFF/></DDD></CCC><CCC><BBB><BBB><BBB/></BBB></BBB></CCC></AAA>");
static const String testXML4("<AAA><BBB>1</BBB><BBB>2</BBB><BBB>3</BBB><BBB>4</BBB></AAA>");

TEST(SPTK_XmlElement, select)
{
    xml::NodeVector elementSet;
    xml::Document   document;

    document.load(testXML1);

    document.select(elementSet, "/AAA");
    EXPECT_EQ(size_t(1), elementSet.size());

    document.select(elementSet, "/AAA/CCC");
    EXPECT_EQ(size_t(2), elementSet.size());

    document.select(elementSet, "/AAA/DDD/BBB");
    EXPECT_EQ(size_t(1), elementSet.size());
}

TEST(SPTK_XmlElement, select2)
{
    xml::NodeVector elementSet;
    xml::Document   document;

    document.load(testXML2);

    document.select(elementSet, "//BBB");
    EXPECT_EQ(size_t(5), elementSet.size());

    document.select(elementSet, "//DDD/BBB");
    EXPECT_EQ(size_t(3), elementSet.size());
}

TEST(SPTK_XmlElement, select3)
{
    xml::NodeVector elementSet;
    xml::Document   document;

    document.load(testXML3);

    document.select(elementSet, "/AAA/CCC/DDD/*");
    EXPECT_EQ(size_t(4), elementSet.size());

    document.select(elementSet, "//*");
    EXPECT_EQ(size_t(17), elementSet.size());
}

TEST(SPTK_XmlElement, select4)
{
    xml::NodeVector elementSet;
    xml::Document   document;

    document.load(testXML4);

    document.select(elementSet, "/AAA/BBB[1]");
    EXPECT_EQ(size_t(1), elementSet.size());
    EXPECT_STREQ("1", elementSet[0]->text().c_str());

    document.select(elementSet, "/AAA/BBB[last()]");
    EXPECT_EQ(size_t(1), elementSet.size());
    EXPECT_STREQ("4", elementSet[0]->text().c_str());
}

TEST(SPTK_XmlElement, select5)
{
    xml::NodeVector elementSet;
    xml::Document   document;

    Buffer buffer;
    buffer.loadFromFile("data/styles.xml");
    document.load(buffer);

    String xpath = "/office:document-styles/office:styles/style:default-style";
    document.select(elementSet, xpath);
    EXPECT_EQ(size_t(4), elementSet.size());

    xpath = "/office:document-styles/office:styles/style:style";
    document.select(elementSet, xpath);
    EXPECT_EQ(size_t(7), elementSet.size());
}

#endif
