/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CXmlNode.h  -  description
                             -------------------
    begin                : Sun May 22 2003
    based on the code    : Mikko Lahteenmaki <Laza@Flashmail.com>
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
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

#ifndef __CXMLNODE_H__
#define __CXMLNODE_H__

#include <sptk5/CBuffer.h>
#include <sptk5/CStrings.h>
#include <sptk5/CDateTime.h>
#include <sptk5/xml/CXmlValue.h>
#include <sptk5/xml/CXmlNodeList.h>

#include <string>
#include <map>
#include <vector>

namespace sptk {

/// @addtogroup XML
/// @{

class CXmlDoc;
class CXmlNode;
class CXmlAttribute;
class CXmlAttributes;

/// @brief XPath Axis enum
enum CXPathAxis
{
    XPA_CHILD,      ///< Child axis
    XPA_DESCENDANT, ///< Descendant axis
    XPA_PARENT      ///< Parent Axis
};

/// @brief Parsed element of XPath
class SP_EXPORT CXPathElement
{
public:
    const std::string*  elementName;     ///< Node name, or '*'
    std::string         criteria;        ///< Criteria
    CXPathAxis          axis;            ///< Axis
    const std::string*  attributeName;   ///< Attribute name (optional)
    std::string         attributeValue;  ///< Attribute value (optional)
    bool                attributeValueDefined; ///< true if attribute value was defined
    int                 nodePosition;    ///< 0 (not required), -1 (last), or node position
public:
    /// @brief Default constructor
    CXPathElement()
    {
        axis = XPA_CHILD;
        nodePosition = 0;
        attributeValueDefined = false;
        elementName = NULL;
        attributeName = NULL;
    }

    /// @brief Copy constructor
    /// @param xpe const CXPathElement&, CXPathElement object to copy from
    CXPathElement(const CXPathElement& xpe)
    {
        elementName = xpe.elementName;
        criteria = xpe.criteria;
        axis = xpe.axis;
        attributeName = xpe.attributeName;
        attributeValue = xpe.attributeValue;
        nodePosition = xpe.nodePosition;
        attributeValueDefined = xpe.attributeValueDefined;
    }
};

/// @brief XML node
///
/// Basic class for any XML node
class SP_EXPORT CXmlNode
{
    friend class CXmlParser;
    friend class CXmlNodeList;
    friend class CXmlDoc;
    friend class CXmlElement;
    friend class CXmlAttribute;
    friend class CXmlAttributes;
public:
    /// @brief Node type enumeration
    enum CXmlNodeType
    {
        DOM_UNDEFINED = 0,      ///< Type isn't defined yet
        DOM_DOCUMENT = 1,       ///< Document node
        DOM_ELEMENT = 2,        ///< Normal element node, can contain subnodes
        DOM_PI = 4,             ///< Processing Instruction node
        DOM_TEXT = 8,           ///< Cdata where all default entities MUST be escaped.
        DOM_CDATA_SECTION = 16, ///< Cdata section, which can contain preformatted char data.
        DOM_COMMENT = 32,       ///< Comment node
        DOM_ATTRIBUTE = 64      ///< Attribute node
    };

    /// @brief CXmlNode own iterator for subnodes
    typedef CXmlNodeList::iterator iterator;

    /// @brief CXmlNode own const_iterator for subnodes
    typedef CXmlNodeList::const_iterator const_iterator;

private:
    /// @brief Sets parent node - for CXmlParser
    void parent(CXmlNode *p);

    /// @brief Checks if any descendent node matches the path element (internal)
    /// @param nodes CXmlNodeVector&, output list of matched nodes
    /// @param pathElements const std::vector<CXPathElement>&, the path elements
    /// @param pathPosition int, current path elements position
    /// @param starPointer const std::string*, the pointer to SST '*' string
    void scanDescendents(CXmlNodeVector& nodes, const std::vector<CXPathElement>& pathElements, int pathPosition,
            const std::string* starPointer);

    /// @brief Checks if the node matches the path element (internal)
    /// @param pathElement const CXPathElement&, the path elements
    /// @param nodePosition int, the position of the node in the matching group
    /// @param starPointer const std::string*, the pointer to SST '*' string
    /// @param nameMatches bool&, (output) true if the node name matches the XPath element
    /// @param positionMatches bool&, (output) true if the node position matches the XPath element
    /// @returns true if node matches the XPath element
    bool matchPathElement(const CXPathElement& pathElement, int nodePosition, const std::string* starPointer, bool& nameMatches,
            bool& positionMatches);

    /// @brief Checks if the node matches the path element (internal)
    /// @param nodes CXmlNodeVector&, output list of matched nodes
    /// @param pathElements const std::vector<CXPathElement>&, the path elements
    /// @param pathPosition int, current path elements position
    /// @param starPointer const std::string*, the pointer to SST '*' string
    void matchNode(CXmlNodeVector& nodes, const std::vector<CXPathElement>& pathElements, int pathPosition,
            const std::string* starPointer);

protected:
    /// @brief Always returns false for CXmlNode since it has no name
    virtual bool nameIs(const std::string* /*sstName*/) const
    {
        return false;
    }

protected:

    CXmlDoc*    m_document;           ///< Parent document pointer
    CXmlNode*   m_parent;             ///< Parent node pointer

    /// @brief Protected constructor - for derived classes
    ///
    /// @param doc CXmlDoc&, node document
    CXmlNode(CXmlDoc& doc)
    {
        m_document = &doc;
        m_parent = 0;
    }

    /// @brief Protected constructor - for derived classes
    ///
    /// @param parent CXmlNode&, node document
    CXmlNode(CXmlNode& parent)
    {
        m_document = parent.document();
        parent.push_back(this);
    }

    /// @brief Destructor
    virtual ~CXmlNode()
    {
        clear();
    }

public:

    /// @brief Finds the first subnode with the given name
    ///
    /// Returns node pointer or NULL, if the node with such name is not found.
    /// @param name std::string, the name to find
    /// @param recursively bool, if true, also search in all subnodes
    CXmlNode* findFirst(std::string name, bool recursively = true) const;

    /// @brief Finds the first subnode with the given name, or creates a new one.
    ///
    /// Returns node pointer. If the node with such name is not found, then new
    /// node is created.
    /// @param name std::string, the name to find
    /// @param recursively bool, if true, also search in all subnodes
    CXmlNode* findOrCreate(std::string name, bool recursively = true);

    /// @brief Finds the first subnode with the given name, or creates a new one.
    ///
    /// Returns node pointer. If the node with such name is not found, then new
    /// node is created. This is a shortcut for findOrCreate(name,false);
    /// @param name std::string, the name to find
    CXmlNode& operator[](std::string name)
    {
        return *findOrCreate(name, false);
    }

    /// @brief Returns node type
    virtual CXmlNodeType type() const = 0;

    /// @brief Selects nodes as defined by XPath
    ///
    /// The implementation is just started, so only limited XPath standard part is supported.
    /// Currently, examples 1 through 6 from http://www.zvon.org/xxl/XPathTutorial/Output/example6.html
    /// are working fine with the exceptions:
    /// - no functions are supported yet.
    /// @param nodes CXmlNodeVector&, the resulting list of subnodes
    /// @param xpath std::string, the xpath for subnodes
    void select(CXmlNodeVector& nodes, std::string xpath);

    /// @brief Performs a deep copy of node and all its subnodes
    ///
    /// @param node const CXmlNode&, a node to copy from
    virtual void copy(const CXmlNode& node);

    /// @brief Deletes all child nodes
    ///
    /// Any memory, associated with child nodes is released.
    virtual void clearChildren()
    {
    }

    /// @brief Deletes all children and clears all the attributes
    ///
    /// Any memory, associated with children or attributes,
    /// is released.
    virtual void clear()
    {
    }

    /// @brief Returns parent node of this node.
    ///
    /// For CXmlDocument this returns 'this' pointer.
    CXmlNode *parent() const
    {
        return m_parent;
    }

    /// @brief Returns document context associated with this node.
    CXmlDoc *document() const
    {
        return m_document;
    }

    /// @brief Returns the node name.
    ///
    /// The meaning of the name depends on the node type:
    virtual const std::string& name() const=0;

    /// @brief Sets the new name for the node
    /// @param name const std::string&, new node name
    virtual void name(const std::string& name)=0;

    /// @brief Sets new name for node
    /// @param name const char *, new node name
    virtual void name(const char *name)=0;

    /// @brief Returns the value of the node
    ///
    /// The meaning of the value depends on the node type.
    /// DOM_DOCUMENT and DOM_ELEMENT don't have values
    virtual const std::string& value() const;

    /// @brief Sets new value to node.
    virtual void value(const std::string& /*new_value*/)
    {
    }

    /// @brief Sets new value to node
    virtual void value(const char* /*new_value*/)
    {
    }

    /// @brief Returns cdatas combined from children.
    ///
    /// E.g. "Some <tag>text</tag> here" becomes: "Some text here"
    std::string text() const;

    /// @brief Sets text for the node.
    ///
    /// First, the node child nodes are removed.
    /// Then, new CXmlNodeText is added to this node.
    /// @param txt std::string, node text
    void text(std::string txt);

    /// @brief Returns referrence to node attributes
    ///
    /// Returns 0 if node isn't CXmlElement or CXmlDocument
    virtual CXmlAttributes& attributes()
    {
        return *(CXmlAttributes*) 0;
    }

    /// @brief Returns referrence to node attributes (const version)
    ///
    /// Returns 0 if node isn't CXmlElement or CXmlDocument
    virtual const CXmlAttributes& attributes() const
    {
        return *(CXmlAttributes*) 0;
    }

    /// @brief Returns true, if node has any attributes
    virtual bool hasAttributes() const
    {
        return false;
    }

    /// @brief Returns true, if given attribute is found
    virtual bool hasAttribute(const char* attr) const
    {
        return false;
    }

    /// @brief Returns attribute value for given attribute.
    ///
    /// HTML tags can have empty attributes, for those you should use has_attribute() method.
    /// @param attr name of attribute
    /// @param defaultValue const char *, a default value. If attribute doesn't exist then default value is returned.
    /// @returns attribute value
    virtual CXmlValue getAttribute(std::string attr, const char *defaultValue = "") const
    {
        return defaultValue;
    }

    /// @brief Sets new value to attribute 'attr'.
    ///
    /// If attribute is not found, it's added to map.
    /// @param attr attribute name
    /// @param value attribute value
    /// @param defaultValue const char *, a default value. If attribute value is matching default value than attribute isn't stored (or removed if it existed).
    virtual void setAttribute(const char *attr, CXmlValue value, const char *defaultValue = "")
    {
    }

    /// @brief Sets new value to attribute 'attr'.
    ///
    /// If attribute is not found, it's added to map.
    /// @param attr const string&, an attribute name
    /// @param value CXmlValue, attribute value
    /// @param defaultValue const char *, a default value. If attribute value is matching default value than attribute isn't stored (or removed if it existed).
    virtual void setAttribute(const std::string& attr, CXmlValue value, const char *defaultValue = "")
    {
    }

    /// @brief Saves node to buffer.
    /// @param buffer to save
    /// @param indent how many indent spaces at start
    virtual void save(CBuffer &buffer, int indent = 0) const;

    /// @brief Returns the first subnode iterator
    virtual iterator begin();

    /// @brief Returns the first subnode const iterator
    virtual const_iterator begin() const;

    /// @brief Returns the end subnode iterator
    virtual iterator end();

    /// @brief Returns the end subnode const iterator
    virtual const_iterator end() const;

    /// @brief Returns a number of subnodes
    virtual uint32_t size() const
    {
        return 0;
    }

    /// @brief Returns true if node has no subnodes of subnodes
    virtual bool empty() const
    {
        return true;
    }

    /// @brief Appends a subnode
    virtual void push_back(CXmlNode* node)
    {
    }

    /// @brief Inserts a subnode
    ///
    /// @param pos iterator, insert position with the list of subnodes
    /// @param node CXmlNode*, node to insert
    virtual void insert(iterator pos, CXmlNode* node)
    {
    }

    /// @brief Removes a subnode
    ///
    /// Release all the allocated memory and disconnects from parent (this node)
    virtual void remove(CXmlNode* node)
    {
    }

    /// @brief Removes a subnode
    ///
    /// Disconnects subnode from parent (this node)
    virtual void unlink(CXmlNode* node)
    {
    }

public:
    /// @brief Document node, can contain subnodes and attributes
    bool isDocument() const
    {
        return type() == DOM_DOCUMENT;
    }

    /// @brief Normal element node or document, can contain subnodes and attributes
    bool isElement() const
    {
        return (type() & (DOM_ELEMENT | DOM_DOCUMENT)) != 0;
    }

    /// @brief Processing Instruction node
    bool isPI() const
    {
        return type() == DOM_PI;
    }

    /// @brief Cdata where all default entities MUST be escaped.
    bool isText() const
    {
        return type() == DOM_TEXT;
    }

    /// @brief Cdata section, which can contain preformatted char data.
    bool isCDataSection() const
    {
        return type() == DOM_CDATA_SECTION;
    }

    /// @brief Comment node
    bool isComment() const
    {
        return type() == DOM_COMMENT;
    }
};

/// @brief XML Named item
///
/// Used as a base class for XML element and XML attribute
class SP_EXPORT CXmlNamedItem: public CXmlNode
{
    friend class CXmlDoc;
    friend class CXmlAttribute;

    const std::string*  m_name;     ///< Node name, stored in the parent document SST

protected:
    /// @brief Protected constructor for creating CXmlDoc only
    ///
    /// @param doc CXmlDoc&, a document.
    CXmlNamedItem(CXmlDoc& doc) :
            CXmlNode(doc)
    {
    }

    /// @brief Returns true if node name pointer (from SST) matches aname pointer
    /// @param sstName const string*, node name pointer to compare with this node name pointer
    virtual bool nameIs(const std::string* sstName) const
    {
        return sstName == m_name;
    }

public:
    /// @brief Constructor
    ///
    /// @param parent CXmlNode*, a parent node.
    /// @param tagname const char*, a name of XML tag
    CXmlNamedItem(CXmlNode& parent, const char* tagname) :
            CXmlNode(parent)
    {
        name(tagname);
    }

    /// @brief Constructor
    ///
    /// @param parent CXmlNode*, a parent node.
    /// @param tagname const char*, a name of XML tag
    CXmlNamedItem(CXmlNode* parent, const char* tagname) :
            CXmlNode(*parent)
    {
        name(tagname);
    }

    /// @brief Constructor
    ///
    /// @param parent CXmlNode &, a parent node.
    /// @param tagname const string&, a name of XML tag
    CXmlNamedItem(CXmlNode& parent, const std::string& tagname) :
            CXmlNode(parent)
    {
        name(tagname);
    }

    /// @brief Returns the node name.
    ///
    /// The meaning of the value depends on the node type
    virtual const std::string& name() const
    {
        return *m_name;
    }

    /// @brief Sets the new name for the node
    /// @param name const std::string&, new node name
    virtual void name(const std::string& name);

    /// @brief Sets new name for node
    /// @param name const char *, new node name
    virtual void name(const char *name);

    /// @brief Returns node type
    virtual CXmlNodeType type() const
    {
        return DOM_ATTRIBUTE;
    }
};

/// @brief Base class for XML nodes with value
class SP_EXPORT CXmlBaseTextNode: public CXmlNode
{
    std::string     m_value;        ///< Node value
protected:
    /// @brief returns node name
    virtual const std::string& nodeName() const;
public:
    /// @brief Constructor
    CXmlBaseTextNode(CXmlNode *parent, const char *data) :
            CXmlNode(*parent)
    {
        value(data);
    }

    /// @brief Returns the value of the node
    ///
    /// The meaning of the value depends on the node type.
    /// DOM_DOCUMENT and DOM_ELEMENT don't have values
    virtual const std::string& value() const
    {
        return m_value;
    }

    /// @brief Sets new value to node.
    ///
    /// @param new_value const std::string &, new value
    /// @see value()
    virtual void value(const std::string &new_value)
    {
        m_value = new_value;
    }

    /// @brief Sets new value to node
    ///
    /// @param new_value const char *, value to set
    /// @see value()
    virtual void value(const char *new_value)
    {
        m_value = new_value;
    }

    /// @brief Returns the node name.
    ///
    /// The meaning of the value depends on the node type
    virtual const std::string& name() const
    {
        return nodeName();
    }

    /// @brief Sets the new name for the node
    /// @param name const std::string&, new node name
    virtual void name(const std::string& name)
    {
    }

    /// @brief Sets new name for node
    /// @param name const char *, new node name
    virtual void name(const char *name)
    {
    }

};

/// @brief XML Text
class SP_EXPORT CXmlText: public CXmlBaseTextNode
{
protected:
    /// @brief returns node name
    virtual const std::string& nodeName() const;
public:
    /// @brief Constructor
    ///
    /// @param parent CXmlNode&, a parent node.
    /// @param data const char*, a text
    CXmlText(CXmlNode& parent, const char *data) :
            CXmlBaseTextNode(&parent, data)
    {
    }

    /// @brief Constructor
    ///
    /// @param parent CXmlNode*, a parent node.
    /// @param data const char*, a text
    CXmlText(CXmlNode* parent, const char *data) :
            CXmlBaseTextNode(parent, data)
    {
    }

    /// @brief Constructor
    ///
    /// @param parent CXmlNode &, a parent node.
    /// @param data const std::string&, a text
    CXmlText(CXmlNode& parent, const std::string& data) :
            CXmlBaseTextNode(&parent, data.c_str())
    {
    }

    /// @brief Returns node type
    virtual CXmlNodeType type() const
    {
        return DOM_TEXT;
    }
};

/// @brief XML comment
class SP_EXPORT CXmlComment: public CXmlBaseTextNode
{
protected:
    /// @brief returns node name
    virtual const std::string& nodeName() const;
public:
    /// @brief Constructor
    ///
    /// @param parent CXmlNode &, a parent node.
    /// @param data const char *, a comment
    CXmlComment(CXmlNode& parent, const char *data) :
            CXmlBaseTextNode(&parent, data)
    {
    }

    /// @brief Constructor
    ///
    /// @param parent CXmlNode*, a parent node.
    /// @param data const char*, a comment
    CXmlComment(CXmlNode* parent, const char* data) :
            CXmlBaseTextNode(parent, data)
    {
    }

    /// @brief Constructor
    ///
    /// @param parent CXmlNode &, a parent node.
    /// @param data const std::string&, a comment
    CXmlComment(CXmlNode& parent, const std::string& data) :
            CXmlBaseTextNode(&parent, data.c_str())
    {
    }

    /// @brief Returns node type
    virtual CXmlNodeType type() const
    {
        return DOM_COMMENT;
    }
};

/// @brief XML CData section
class SP_EXPORT CXmlCDataSection: public CXmlBaseTextNode
{
protected:
    /// @brief returns node name
    virtual const std::string& nodeName() const;
public:
    /// @brief Constructor
    ///
    /// @param parent CXmlNode&, a parent node.
    /// @param data const char*, a data
    CXmlCDataSection(CXmlNode& parent, const char* data) :
            CXmlBaseTextNode(&parent, data)
    {
    }

    /// @brief Constructor
    ///
    /// @param parent CXmlNode*, a parent node.
    /// @param data const char*, a data
    CXmlCDataSection(CXmlNode* parent, const char* data) :
            CXmlBaseTextNode(parent, data)
    {
    }
    /// @brief Constructor
    ///
    /// @param parent CXmlNode &, a parent node.
    /// @param data const std::string&, a data
    CXmlCDataSection(CXmlNode& parent, const std::string& data) :
            CXmlBaseTextNode(&parent, data.c_str())
    {
    }

    /// @brief Returns node type
    virtual CXmlNodeType type() const
    {
        return DOM_CDATA_SECTION;
    }
};

/// XML processing instructions (PI)
class SP_EXPORT CXmlPI: public CXmlBaseTextNode
{
    const std::string*  m_name;     ///< Node name, stored in the parent document SST
public:
    /// @brief Constructor
    ///
    /// @param parent CXmlNode &, a parent node. Make sure it's a pointer to the existing node.
    /// @param target std::string, a target tag name
    /// @param data const char *, a data
    CXmlPI(CXmlNode& parent, std::string target, const char *data) :
            CXmlBaseTextNode(&parent, data)
    {
        name(target);
    }

    /// @brief Constructor
    ///
    /// @param parent CXmlNode&, a parent node. Make sure it's a pointer to the existing node.
    /// @param target std::string, a target tag name
    /// @param data const char*, a data
    CXmlPI(CXmlNode* parent, std::string target, const char* data) :
            CXmlBaseTextNode(parent, data)
    {
        name(target);
    }

    /// @brief Constructor
    ///
    /// @param parent CXmlNode &, a parent node
    /// @param target std::string, a target tag name
    /// @param data const std::string&, a data
    CXmlPI(CXmlNode& parent, std::string target, const std::string& data) :
            CXmlBaseTextNode(&parent, data.c_str())
    {
        name(target);
    }

    /// @brief Returns the node name.
    ///
    /// The meaning of the value depends on the node type
    virtual const std::string& name() const
    {
        return *m_name;
    }

    /// @brief Sets the new name for the node
    /// @param name const std::string&, new node name
    virtual void name(const std::string& name);

    /// @brief Sets new name for node
    /// @param name const char *, new node name
    virtual void name(const char *name);

    /// @brief Returns node type
    virtual CXmlNodeType type() const
    {
        return DOM_PI;
    }
};

#define XML_TYPE_DOC             CXmlNode::DOM_DOCUMENT         ///< Node type is a document
#define XML_TYPE_ELEMENT         CXmlNode::DOM_ELEMENT          ///< Node type is an element
#define XML_TYPE_PI              CXmlNode::DOM_PI               ///< Node type is a processing instruction
#define XML_TYPE_TEXT            CXmlNode::DOM_TEXT             ///< Node type is a text
#define XML_TYPE_CDATA_SECTION   CXmlNode::DOM_CDATA_SECTION    ///< Node type is CDataSection
#define XML_TYPE_COMMENT         CXmlNode::DOM_COMMENT          ///< Node type is a comment
/// @}
}
#endif
