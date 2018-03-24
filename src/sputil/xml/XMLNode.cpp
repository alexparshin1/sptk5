/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       XMLNode.cpp - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2017 by Alexey Parshin. All rights reserved.  ║
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

using namespace std;

namespace sptk {

    /// An empty string to use as a stub for value()
    static const String emptyString;
    static const string indentsString(1024, ' ');

    /// An empty nodes set to emulate a set of stub iterators
    static XMLNodeList emptyNodes;

    const String& XMLNode::value() const
    {
        return emptyString;
    }

    XMLNode::iterator XMLNode::begin()
    {
        return emptyNodes.end();
    }

    XMLNode::const_iterator XMLNode::begin() const
    {
        return emptyNodes.end();
    }

    XMLNode::iterator XMLNode::end()
    {
        return emptyNodes.end();
    }

    XMLNode::const_iterator XMLNode::end() const
    {
        return emptyNodes.end();
    }

    void XMLNode::parent(XMLNode *p)
    {
        if (m_parent == p)
            return;
        if (m_parent != nullptr)
            m_parent->unlink(p);

        m_parent = p;

        if (m_parent != nullptr)
            m_parent->push_back(this);
    }

    static void parsePathElement(XMLDocument* document, const string& pathElementStr, XPathElement& pathElement)
    {
        pathElement.elementName = nullptr;
        pathElement.attributeName = nullptr;
        pathElement.axis = XPA_CHILD;
        size_t backBracketPosition = pathElementStr.rfind(']');
        string pathElementName;
        if (backBracketPosition == STRING_NPOS) {
            pathElementName = pathElementStr;
            pathElement.criteria.clear();
        }
        else {
            size_t bracketPosition = pathElementStr.find('[');
            if (bracketPosition == STRING_NPOS || backBracketPosition < bracketPosition) {
                pathElementName = pathElementStr;
                pathElement.criteria.clear();
            }
            else {
                pathElementName = pathElementStr.substr(0, bracketPosition);
                pathElement.criteria = pathElementStr.substr(bracketPosition + 1, backBracketPosition - bracketPosition - 1);
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
            pathElement.attributeName = &document->shareString(pathElementName.c_str() + 1);
        else
            pathElement.elementName = &document->shareString(pathElementName.c_str());

        string& criteria = pathElement.criteria;

        if (!criteria.empty()) {
            int& nodePosition = pathElement.nodePosition;
            nodePosition = string2int(pathElement.criteria);
            if (nodePosition == 0 && criteria == "last()")
                nodePosition = -1;

            if (nodePosition == 0) {
                if (criteria[0] == '@') {
                    pos = criteria.find('=');
                    if (pos == STRING_NPOS)
                        pathElement.attributeName = &document->shareString(criteria.c_str() + 1);
                    else {
                        pathElement.attributeName = &document->shareString(criteria.substr(1, pos - 1));
                        if (criteria[pos + 1] == '\'' || criteria[pos + 1] == '"')
                            pathElement.attributeValue = criteria.substr(pos + 2, criteria.length() - (pos + 3));
                        else
                            pathElement.attributeValue = criteria.substr(pos + 1, criteria.length() - (pos + 1));
                        pathElement.attributeValueDefined = true;
                    }
                }
            }
        }
    }

    bool XMLNode::matchPathElement(const XPathElement& pathElement, int nodePosition, const string* starPointer,
        bool& nameMatches, bool& positionMatches)
    {
        if (pathElement.elementName != nullptr && pathElement.elementName != starPointer && !nameIs(pathElement.elementName))
            return false;

        // Node criteria is position
        if (pathElement.nodePosition != 0) {
            positionMatches = pathElement.nodePosition == nodePosition;
            return positionMatches;
        }

        // Node criteria is attribute
        if (pathElement.attributeName != nullptr && type() == DOM_ELEMENT) {
            XMLAttributes& attributes = this->attributes();
            bool attributeMatch = false;
            if (pathElement.attributeValueDefined) {
                if (pathElement.attributeName == starPointer) {
                    for (XMLAttributes::const_iterator attr = attributes.begin(); attr != attributes.end(); ++attr) {
                        XMLNode* a = *attr;
                        if (a->name() == pathElement.attributeValue) {
                            attributeMatch = true;
                            break;
                        }
                    }
                }
                else
                    attributeMatch = attributes.getAttribute(*pathElement.attributeName).str() == pathElement.attributeValue;
            }
            else {
                if (pathElement.attributeName == starPointer)
                    attributeMatch = hasAttributes();
                else
                    attributeMatch = hasAttribute(pathElement.attributeName->c_str());
            }
            return attributeMatch;
        }
        return true;
    }

    void XMLNode::scanDescendents(XMLNodeVector& nodes, const std::vector<XPathElement>& pathElements, int pathPosition,
        const std::string* starPointer)
    {
        const XPathElement& pathElement = pathElements[size_t(pathPosition)];
        XMLNode* lastNode = nullptr;
        int currentPosition = 1;
        for (auto node: *this) {
            bool nameMatches = false;
            bool positionMatches = false;
            if (node->matchPathElement(pathElement, currentPosition, starPointer, nameMatches, positionMatches)) {
                currentPosition++;
                node->matchNode(nodes, pathElements, pathPosition, starPointer);
            }
            else {
                if (nameMatches)
                    currentPosition++;
                if (pathElement.nodePosition < 0 && nameMatches)
                    lastNode = node;
            }
            if ((node->type() & (DOM_DOCUMENT | DOM_ELEMENT)) != 0)
                node->scanDescendents(nodes, pathElements, pathPosition, starPointer);
        }
        if (lastNode != nullptr)
            lastNode->matchNode(nodes, pathElements, pathPosition, starPointer);
    }

    void XMLNode::matchNode(XMLNodeVector& nodes, const vector<XPathElement>& pathElements, int pathPosition, const std::string* starPointer)
    {
        pathPosition++;
        if (pathPosition == (int)pathElements.size()) {
            const XPathElement& pathElement = pathElements[size_t(pathPosition - 1)];
            if (pathElement.elementName != nullptr)
                nodes.insert(nodes.end(), this);
            else if (pathElement.attributeName != nullptr) {
                XMLAttribute* attributeNode = attributes().getAttributeNode(*pathElement.attributeName);
                if (attributeNode != nullptr)
                    nodes.insert(nodes.end(), dynamic_cast<XMLNode*>(attributeNode));
            }
            return;
        }

        const XPathElement& pathElement = pathElements[size_t(pathPosition)];

        XMLNode* lastNode = nullptr;
        int currentPosition = 1;
        for (auto node: *this) {
            bool nameMatches;
            bool positionMatches;
            //string nodeName = node->name();
            if (node->matchPathElement(pathElement, currentPosition, starPointer, nameMatches, positionMatches)) {
                currentPosition++;
                node->matchNode(nodes, pathElements, pathPosition, starPointer);
            }
            else if (pathElement.nodePosition < 0 && nameMatches)
                lastNode = node;
            if (pathElement.axis == XPA_DESCENDANT)
                node->scanDescendents(nodes, pathElements, pathPosition, starPointer);
        }
        if (lastNode != nullptr)
            lastNode->matchNode(nodes, pathElements, pathPosition, starPointer);
    }

    void XMLNode::select(XMLNodeVector& nodes, String xpath)
    {
        const char* ptr;
        nodes.clear();

        xpath = xpath.replace("\\/\\/", "/descendant::");

        if (xpath[0] == '/')
            ptr = xpath.c_str() + 1;
        else
            ptr = xpath.c_str();

        Strings pathElementStrs(ptr, "/");
        vector<XPathElement> pathElements(pathElementStrs.size());
        for (unsigned i = 0; i < pathElements.size(); i++)
            parsePathElement(document(), pathElementStrs[i], pathElements[i]);

        const string* starPointer = &document()->shareString("*");
        matchNode(nodes, pathElements, -1, starPointer);
    }

    void XMLNode::copy(const XMLNode& node)
    {
        name(node.name());
        value(node.value());
        if (isElement() && node.isElement()) {
            attributes() = node.attributes();
        }

        for (auto childNode: node) {
            XMLNode* element;
            switch (childNode->type())
            {
            case DOM_ELEMENT:
                element = new XMLElement(this, "");
                element->copy(*childNode);
                break;
            case DOM_PI:
                new XMLPI(*this, childNode->name(), childNode->value());
                break;
            case DOM_TEXT:
                new XMLText(*this, childNode->value());
                break;
            case DOM_CDATA_SECTION:
                new XMLCDataSection(*this, childNode->value());
                break;
            case DOM_COMMENT:
                new XMLComment(*this, childNode->value());
                break;
            default:
                break;
            }
        }
    }

    String XMLNode::text() const
    {
        String ret;

        if ((type() & (DOM_TEXT | DOM_CDATA_SECTION)) != 0)
            ret += value();
        else {
            for (auto np: *this) {
                if ((np->type() & (DOM_TEXT | DOM_CDATA_SECTION)) != 0)
                    ret += np->value();
            }
        }

        return ret;
    }

    void XMLNode::text(const String& txt)
    {
        clearChildren();
        new XMLText(*this, txt);
    }

    void XMLNode::save(Buffer &buffer, int indent) const
    {
        // output indendation spaces
        if (indent > 0)
            buffer.append(indentsString.c_str(), size_t(indent));

        const string& nodeName = name();

        if (type() == DOM_ELEMENT) {
            // Output tag name
            buffer.append('<');
            buffer.append(nodeName);
            const XMLAttributes& attributes = this->attributes();
            if (!attributes.empty()) {
                // Output attributes
                Buffer real_id, real_val;
                for (auto attributeNode: attributes) {
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
        }

        // depending on the nodetype, do output
        switch (type())
        {
        case DOM_PI:
            buffer.append("<?" + std::string(name()) + " " + value() + "?>\n");
            break;

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

        case DOM_ELEMENT: {
            if (!empty()) {
                bool only_cdata;
                XMLNode *nd = *begin();
                if (size() == 1 && nd->type() == DOM_TEXT) {
                    only_cdata = true;
                    buffer.append('>');
                }
                else {
                    only_cdata = false;
                    buffer.append(">\n", 2);
                }

                // output all subnodes
                for (auto np: *this) {
                    if (only_cdata)
                        np->save(buffer, -1);
                    else {
                        np->save(buffer, indent + m_document->indentSpaces());
                        if (buffer.data()[buffer.bytes() - 1] != '\n')
                            buffer.append(char('\n'));
                    }
                }

                // output indendation spaces
                if (!only_cdata && indent > 0)
                    buffer.append(indentsString.c_str(), size_t(indent));

                // output closing tag
                buffer.append("</", 2);
                buffer.append(name());
                buffer.append(">\n", 2);

            }
            else {
                //LEAF
                buffer.append("/>\n", 3);
            }
        }
            break;

        default: // unknown nodetype
            break;
        }
    }

    void XMLNode::save(json::Element& json, string& text) const
    {
        const string& nodeName = name();

        if (isText()) {
            text.append(this->text());
            return;
        }

        if (isPI()) {
            json.add(name(), value());
            return;
        }

        auto object = new json::Element(new json::ObjectData());
        json.add(nodeName, object);

        if (isElement()) {
            const XMLAttributes& attributes = this->attributes();
            if (!attributes.empty()) {
                auto attrs = new json::Element(new json::ObjectData());
                object->add("attributes", attrs);
                for (auto attributeNode: attributes)
                    attrs->add(attributeNode->name(), attributeNode->value());
            }
        }

        switch (type())
        {
            case DOM_TEXT:
            case DOM_CDATA_SECTION:
                if (value().substr(0, 9) == "<![CDATA[" && value().substr(value().length() - 3) == "]]>")
                    *object = json::Element(value().substr(9, value().length() - 12));
                else
                    *object = json::Element(value());
                break;

            case DOM_COMMENT:
                // output all subnodes
                object->add("comments", value());
                break;

            case DOM_ELEMENT: {
                if (empty()) {
                    *object = json::Element("");
                } else {
                    bool done = false;
                    if (size() == 1) {
                        for (auto np: *this)
                            if (np->name() == "null") {
                                *object = json::Element();
                                done = true;
                            }
                    }
                    if (!done) {
                        // output all subnodes
                        string nodeText;
                        for (auto np: *this)
                            np->save(*object, nodeText);
                        if (object->isObject() && object->size() == 0) {
                            if (m_document->m_matchNumber.matches(nodeText)) {
                                double value = string2double(nodeText);
                                *object = json::Element(value);
                            } else
                                *object = json::Element(nodeText);
                        }
                    }
                }
            }
                break;

            default: // unknown nodetype
                break;
        }
    }

    void XMLNode::save(json::Document& json) const
    {
        string text;
        for (auto np: *this)
            np->save(json.root(), text);
    }

    XMLNode *XMLNode::findFirst(const std::string& aname, bool recursively) const
    {
        for (auto node: *this) {
            if (node->name() == aname)
                return node;
            if (recursively && !node->empty()) {
                XMLNode *cnode = node->findFirst(aname, true);
                if (cnode != nullptr)
                    return cnode;
            }
        }
        return nullptr;
    }

    XMLNode *XMLNode::findOrCreate(const std::string& aname, bool recursively)
    {
        XMLNode *node = findFirst(aname, recursively);
        if (node != nullptr)
            return node;
        return new XMLElement(*this, aname);
    }

    const std::string& XMLBaseTextNode::nodeName() const
    {
        return emptyString;
    }

    void XMLNamedItem::name(const std::string& name)
    {
        m_name = &document()->shareString(name.c_str());
    }

    void XMLNamedItem::name(const char *name)
    {
        m_name = &document()->shareString(name);
    }

    void XMLElement::insert(iterator itor, XMLNode* node)
    {
        m_nodes.insert(itor, node);
        node->m_parent = this;
    }

    void XMLElement::push_back(XMLNode* node)
    {
        m_nodes.insert(m_nodes.end(), node);
        node->m_parent = this;
    }

    void XMLElement::unlink(XMLNode* node)
    {
        auto itor = find(begin(), end(), node);
        if (itor == end())
            return;
        m_nodes.erase(itor);
    }

    void XMLElement::remove(XMLNode* node)
    {
        auto itor = find(begin(), end(), node);
        if (itor == end())
            return;
        delete *itor;
        m_nodes.erase(itor);
    }

    void XMLElement::clearChildren()
    {
        m_nodes.clear();
    }

    void XMLElement::clear()
    {
        XMLNode::clear();
        m_nodes.clear();
        m_attributes.clear();
    }

    void XMLPI::name(const std::string& name)
    {
        m_name = &document()->shareString(name.c_str());
    }

    void XMLPI::name(const char *name)
    {
        m_name = &document()->shareString(name);
    }

    const std::string& XMLComment::nodeName() const
    {
        static const string nodeNameString("#comment");
        return nodeNameString;
    }

    const std::string& XMLText::nodeName() const
    {
        static const string nodeNameString("#text");
        return nodeNameString;
    }

    const std::string& XMLCDataSection::nodeName() const
    {
        static const string nodeNameString("#cdata-section");
        return nodeNameString;
    }
}
