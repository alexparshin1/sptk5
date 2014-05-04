/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CXmlNode.cpp  -  description
                             -------------------
    begin                : Sun May 22 2003
    based on the code    : Mikko Lahteenmaki <Laza@Flashmail.com>
    copyright            : (C) 1999-2013 by Alexey S.Parshin
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#include <sptk5/CException.h>
#include <sptk5/cxml>
#include <algorithm>

using namespace std;
using namespace sptk;

/// An empty string to use as a stub for value()
static const std::string emptyString;
static const string indentsString(1024, ' ');

/// An empty nodes set to emulate a set of stub iterators
static CXmlNodeList emptyNodes;

const std::string& CXmlNode::value() const
{
    return emptyString;
}

CXmlNode::iterator CXmlNode::begin()
{
    return emptyNodes.end();
}

CXmlNode::const_iterator CXmlNode::begin() const
{
    return emptyNodes.end();
}

CXmlNode::iterator CXmlNode::end()
{
    return emptyNodes.end();
}

CXmlNode::const_iterator CXmlNode::end() const
{
    return emptyNodes.end();
}

void CXmlNode::parent(CXmlNode *p)
{
    if (m_parent == p)
        return;
    if (m_parent)
        m_parent->unlink(p);

    m_parent = p;

    if (m_parent)
        m_parent->push_back(this);
}

static void parsePathElement(CXmlDoc* document, const string& pathElementStr, CXPathElement& pathElement)
{
    pathElement.elementName = NULL;
    pathElement.attributeName = NULL;
    pathElement.axis = XPA_CHILD;
    size_t backBracketPosition = pathElementStr.rfind("]");
    string pathElementName;
    if (backBracketPosition == STRING_NPOS) {
        pathElementName = pathElementStr;
        pathElement.criteria.clear();
    } else {
        size_t bracketPosition = pathElementStr.find("[");
        if (bracketPosition == STRING_NPOS || backBracketPosition < bracketPosition) {
            pathElementName = pathElementStr;
            pathElement.criteria.clear();
        } else {
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
    int& nodePosition = pathElement.nodePosition;

    if (!criteria.empty()) {
        pathElement.nodePosition = string2int(pathElement.criteria);
        if (!nodePosition && criteria == "last()")
            nodePosition = -1;

        if (nodePosition == 0) {
            if (criteria[0] == '@') {
                size_t pos = criteria.find("=");
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

bool CXmlNode::matchPathElement(const CXPathElement& pathElement, int nodePosition, const string* starPointer,
        bool& nameMatches, bool& positionMatches)
{
    if (pathElement.elementName && pathElement.elementName != starPointer && !nameIs(pathElement.elementName))
        return false;

    // Node criteria is position
    if (pathElement.nodePosition) {
        positionMatches = pathElement.nodePosition == nodePosition;
        return positionMatches;
    }
    // Node criteria is attribute
    else if (pathElement.attributeName && type() == DOM_ELEMENT) {
        CXmlAttributes& attributes = this->attributes();
        bool attributeMatch = false;
        if (pathElement.attributeValueDefined) {
            if (pathElement.attributeName == starPointer) {
                for (CXmlAttributes::const_iterator attr = attributes.begin(); attr != attributes.end(); attr++) {
                    CXmlNode* a = *attr;
                    if (a->name() == pathElement.attributeValue) {
                        attributeMatch = true;
                        break;
                    }
                }
            } else
                attributeMatch = attributes.getAttribute(*pathElement.attributeName).str() == pathElement.attributeValue;
        } else {
            if (pathElement.attributeName == starPointer)
                attributeMatch = hasAttributes();
            else
                attributeMatch = hasAttribute(pathElement.attributeName->c_str());
        }
        return attributeMatch;
    } else
        return true;
#ifdef __MSCVER__
    return false;
#endif
}

void CXmlNode::scanDescendents(CXmlNodeVector& nodes, const std::vector<CXPathElement>& pathElements, int pathPosition,
        const std::string* starPointer)
{
    const CXPathElement& pathElement = pathElements[size_t(pathPosition)];
    CXmlNode* lastNode = 0;
    int currentPosition = 1;
    for (iterator itor = begin(); itor != end(); itor++) {
        CXmlNode* node = *itor;
        bool nameMatches;
        bool positionMatches;
        if (node->matchPathElement(pathElement, currentPosition, starPointer, nameMatches, positionMatches)) {
            currentPosition++;
            node->matchNode(nodes, pathElements, pathPosition, starPointer);
        } else {
            if (nameMatches)
                currentPosition++;
            if (pathElement.nodePosition < 0 && nameMatches)
                lastNode = node;
        }
        if (node->type() & (DOM_DOCUMENT | DOM_ELEMENT))
            node->scanDescendents(nodes, pathElements, pathPosition, starPointer);
    }
    if (lastNode)
        lastNode->matchNode(nodes, pathElements, pathPosition, starPointer);
}

void CXmlNode::matchNode(CXmlNodeVector& nodes, const vector<CXPathElement>& pathElements, int pathPosition,
        const std::string* starPointer)
{
    pathPosition++;
    if (pathPosition == (int) pathElements.size()) {
        const CXPathElement& pathElement = pathElements[size_t(pathPosition - 1)];
        if (pathElement.elementName)
            nodes.insert(nodes.end(), this);
        else if (pathElement.attributeName) {
            CXmlAttribute* attributeNode = attributes().getAttributeNode(*pathElement.attributeName);
            if (attributeNode)
                nodes.insert(nodes.end(), attributeNode);
        }
        return;
    }

    const CXPathElement& pathElement = pathElements[size_t(pathPosition)];

    CXmlNode* lastNode = 0;
    int currentPosition = 1;
    for (iterator itor = begin(); itor != end(); itor++) {
        CXmlNode* node = *itor;
        bool nameMatches;
        bool positionMatches;
        string nodeName = node->name();
        if (node->matchPathElement(pathElement, currentPosition, starPointer, nameMatches, positionMatches)) {
            currentPosition++;
            node->matchNode(nodes, pathElements, pathPosition, starPointer);
        } else if (pathElement.nodePosition < 0 && nameMatches)
            lastNode = node;
        if (pathElement.axis == XPA_DESCENDANT)
            node->scanDescendents(nodes, pathElements, pathPosition, starPointer);
    }
    if (lastNode)
        lastNode->matchNode(nodes, pathElements, pathPosition, starPointer);
}

void CXmlNode::select(CXmlNodeVector& nodes, string xpath)
{
    const char* ptr;
    nodes.clear();

    xpath = replaceAll(xpath, "//", "/descendant::");

    if (xpath[0] == '/')
        ptr = xpath.c_str() + 1;
    else
        ptr = xpath.c_str();

    CStrings pathElementStrs(ptr, "/");
    vector<CXPathElement> pathElements(pathElementStrs.size());
    for (unsigned i = 0; i < pathElements.size(); i++)
        parsePathElement(document(), pathElementStrs[i], pathElements[i]);

    const string* starPointer = &document()->shareString("*");
    matchNode(nodes, pathElements, -1, starPointer);
}

void CXmlNode::copy(const CXmlNode& node)
{
    name(node.name());
    value(node.value());
    if (isElement() && node.isElement()) {
        attributes() = node.attributes();
    }

    const_iterator itor = node.begin();
    const_iterator iend = node.end();
    for (; itor != iend; itor++) {
        const CXmlNode* node = *itor;
        switch (node->type())
        {
        case DOM_ELEMENT: {
            CXmlNode* element = new CXmlElement(this, "");
            element->copy(*node);
        }
            break;
        case DOM_PI:
            new CXmlPI(*this, node->name(), node->value());
            break;
        case DOM_TEXT:
            new CXmlText(*this, node->value());
            break;
        case DOM_CDATA_SECTION:
            new CXmlCDataSection(*this, node->value());
            break;
        case DOM_COMMENT:
            new CXmlComment(*this, node->value());
            break;
        default:
            break;
        }
    }
}

std::string CXmlNode::text() const
{
    std::string ret;

    if (type() & (DOM_TEXT | DOM_CDATA_SECTION))
        ret += value();
    else {
        const_iterator itor = begin();
        const_iterator iend = end();
        for (; itor != iend; itor++) {
            CXmlNode *np = *itor;
            if (np->type() & (DOM_TEXT | DOM_CDATA_SECTION))
                ret += np->value();
        }
    }

    return ret;
}

void CXmlNode::text(std::string txt)
{
    clearChildren();
    new CXmlText(*this, txt);
}

void CXmlNode::save(CBuffer &buffer, int indent) const
{
    // output indendation spaces
    if (indent > 0)
        buffer.append(indentsString.c_str(), size_t(indent));

    if (type() == DOM_ELEMENT) {
        // Output tag name
        buffer.append('<');
        buffer.append(name());
        const CXmlAttributes& attributes = this->attributes();
        if (attributes.size()) {
            // Output attributes
            CBuffer real_id, real_val;
            for (CXmlAttributes::const_iterator it = attributes.begin(); it != attributes.end(); it++) {
                CXmlNode* attributeNode = *it;
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
        if (size()) {
            bool only_cdata;
            const_iterator itor = begin();
            const_iterator iend = end();
            CXmlNode *nd = *itor;
            if (size() == 1 && nd->type() == DOM_TEXT) {
                only_cdata = true;
                buffer.append('>');
            } else {
                only_cdata = false;
                buffer.append(">\n", 2);
            }

            // output all subnodes
            for (; itor != iend; itor++) {
                CXmlNode *np = *itor;
                if (only_cdata)
                    np->save(buffer, -1);
                else {
                    np->save(buffer, indent + m_document->indentSpaces());
                    if (buffer.data()[buffer.bytes() - 1] != '\n')
                        buffer.append('\n');
                }
            }
            // output indendation spaces
            if (!only_cdata && indent > 0)
                buffer.append(indentsString.c_str(), size_t(indent));

            // output closing tag
            buffer.append("</", 2);
            buffer.append(name());
            buffer.append(">\n", 2);

        } else {
            //LEAF
            buffer.append("/>\n", 3);
        }
    }
        break;

    default: // unknown nodetype
        break;
    }
}

CXmlNode *CXmlNode::findFirst(std::string aname, bool recursively) const
{
    for (const_iterator itor = begin(); itor != end(); itor++) {
        CXmlNode *node = *itor;
        if (node->name() == aname)
            return node;
        if (recursively && node->size()) {
            CXmlNode *cnode = node->findFirst(aname, true);
            if (cnode)
                return cnode;
        }
    }
    return 0L;
}

CXmlNode *CXmlNode::findOrCreate(std::string aname, bool recursively)
{
    CXmlNode *node = findFirst(aname, recursively);
    if (node)
        return node;
    return new CXmlElement(*this, aname);
}

const std::string& CXmlBaseTextNode::nodeName() const
{
    return emptyString;
}

void CXmlNamedItem::name(const std::string& name)
{
    m_name = &document()->shareString(name.c_str());
}

void CXmlNamedItem::name(const char *name)
{
    m_name = &document()->shareString(name);
}

void CXmlElement::insert(iterator itor, CXmlNode* node)
{
    m_nodes.insert(itor, node);
    node->m_parent = this;
}

void CXmlElement::push_back(CXmlNode* node)
{
    m_nodes.insert(m_nodes.end(), node);
    node->m_parent = this;
}

void CXmlElement::unlink(CXmlNode* node)
{
    iterator itor = find(begin(), end(), node);
    if (itor == end())
        return;
    m_nodes.erase(itor);
}

void CXmlElement::remove(CXmlNode* node)
{
    iterator itor = find(begin(), end(), node);
    if (itor == end())
        return;
    delete *itor;
    m_nodes.erase(itor);
}

void CXmlElement::clearChildren()
{
    m_nodes.clear();
}

void CXmlElement::clear()
{
    CXmlNode::clear();
    m_nodes.clear();
    m_attributes.clear();
}

void CXmlPI::name(const std::string& name)
{
    m_name = &document()->shareString(name.c_str());
}

void CXmlPI::name(const char *name)
{
    m_name = &document()->shareString(name);
}

const std::string& CXmlComment::nodeName() const
{
    static const string nodeNameString("#comment");
    return nodeNameString;
}

const std::string& CXmlText::nodeName() const
{
    static const string nodeNameString("#text");
    return nodeNameString;
}

const std::string& CXmlCDataSection::nodeName() const
{
    static const string nodeNameString("#cdata-section");
    return nodeNameString;
}
