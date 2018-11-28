/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       Node.h - description                                   ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_XML_NODE_H__
#define __SPTK_XML_NODE_H__

#include <sptk5/Buffer.h>
#include <sptk5/Strings.h>
#include <sptk5/DateTime.h>
#include <sptk5/xml/Value.h>
#include <sptk5/xml/NodeList.h>

#include <string>
#include <map>
#include <vector>

namespace sptk {

namespace json {
class Document;
class Element;
}

namespace xml {

/**
 * @addtogroup XML
 * @{
 */

class Document;
class Attribute;
class Attributes;

/**
 * XPath Axis enum
 */
enum XPathAxis
{
    /**
     * Child axis
     */
            XPA_CHILD,

    /**
     * Descendant axis
     */
            XPA_DESCENDANT,

    /**
     * Parent Axis
     */
            XPA_PARENT

};

/**
 * Parsed element of XPath
 */
class SP_EXPORT XPathElement
{
public:
    /**
     * Node name, or '*'
     */
    const std::string* elementName{ nullptr };

    /**
     * Criteria
     */
    std::string criteria;

    /**
     * Axis
     */
    XPathAxis axis {XPA_CHILD};

    /**
     * Attribute name (optional)
     */
    const std::string* attributeName{ nullptr };

    /**
     * Attribute value (optional)
     */
    std::string attributeValue;

    /**
     * true if attribute value was defined
     */
    bool attributeValueDefined {0};

    /**
     * 0 (not required), -1 (last), or node position
     */
    int nodePosition {0};

    /**
     * Default constructor
     */
    XPathElement()
    {
        axis = XPA_CHILD;
        attributeValueDefined = false;
    }

    /**
     * Copy constructor
     * @param xpe CXPathElement object to copy from
     */
    XPathElement(const XPathElement& xpe)
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

/**
 * XML node
 *
 * Basic class for any XML node
 */
class SP_EXPORT Node
{
    friend class Parser;
    friend class NodeList;
    friend class Document;
    friend class Element;
    friend class Attribute;
    friend class Attributes;
    friend class NodeSearchAlgorithms;
public:
    /**
     * Node type enumeration
     */
    enum NodeType
    {
        /**
         * Type isn't defined yet
         */
        DOM_UNDEFINED = 0,

        /**
         * Document node
         */
        DOM_DOCUMENT = 1,

        /**
         * Normal element node, can contain subnodes
         */
        DOM_ELEMENT = 2,

        /**
         * Processing Instruction node
         */
        DOM_PI = 4,

        /**
         * Cdata where all default entities MUST be escaped.
         */
        DOM_TEXT = 8,

        /**
         * Cdata section, which can contain preformatted char data.
         */
        DOM_CDATA_SECTION = 16,

        /**
         * Comment node
         */
        DOM_COMMENT = 32,

        /**
         * Attribute node
         */
        DOM_ATTRIBUTE = 64
    };

    /**
     * xml::Node own iterator for subnodes
     */
    typedef NodeList::iterator iterator;

    /**
     * xml::Node own const_iterator for subnodes
     */
    typedef NodeList::const_iterator const_iterator;

private:
    /**
     * Sets parent node - for XMLParser
     */
    void parent(Node* p);

    /**
     * Save node to JSON object.
     * @param json              JSON element
     * @param text              Temporary text buffer
     */
    virtual void save(json::Element& json, std::string& text) const;

    /**
     * Parent document pointer
     */
    Document* m_document;

    /**
     * Parent node pointer
     */
    Node* m_parent {nullptr};

protected:

    /**
     * Always returns false for xml::Node since it has no name
     */
    virtual bool nameIs(const std::string* /*sstName*/) const
    {
        return false;
    }

    /**
     * Protected constructor - for derived classes
     *
     * @param doc               Node document
     */
    explicit Node(Document& doc)
    : m_document(&doc)
    {}

    /**
     * Protected constructor - for derived classes
     *
     * @param parent            Node document
     */
    explicit Node(Node& parent)
    : m_document(parent.document())
    {
        parent.push_back(this);
    }

    /**
     * Destructor
     */
    virtual ~Node()
    {
    }

public:

    /**
     * Finds the first subnode with the given name
     *
     * Returns node             Pointer or NULL, if the node with such name is not found.
     * @param name              The name to find
     * @param recursively       If true, also search in all subnodes
     */
    Node* findFirst(const std::string& name, bool recursively = true) const;

    /**
     * Finds the first subnode with the given name, or creates a new one.
     *
     * Returns node pointer. If the node with such name is not found, then new
     * node is created.
     * @param name              The name to find
     * @param recursively       If true, also search in all subnodes
     */
    Node* findOrCreate(const std::string& name, bool recursively = true);

    /**
     * Finds the first subnode with the given name, or creates a new one.
     *
     * Returns node pointer. If the node with such name is not found, then new
     * node is created.
     * @param name              The name to find
     */
    Node& operator[](const std::string& name)
    {
        return *findOrCreate(name, false);
    }

    /**
     * Returns node type
     */
    virtual NodeType type() const = 0;

    /**
     * Selects nodes as defined by XPath
     *
     * The implementation is just started, so only limited XPath standard part is supported.
     * Currently, examples 1 through 6 from http://www.zvon.org/xxl/XPathTutorial/Output/example1.html
     * are working fine with the exceptions:
     * - no functions are supported yet.
     * @param nodes             The resulting list of subnodes
     * @param xpath             The xpath for subnodes
     */
    void select(NodeVector& nodes, String xpath);

    /**
     * Performs a deep copy of node and all its subnodes
     *
     * @param node              Node to copy from
     */
    virtual void copy(const Node& node);

    /**
     * Deletes all child nodes
     *
     * Any memory, associated with child nodes is released.
     */
    virtual void clearChildren()
    {
        // Implement in derived classes
    }

    /**
     * Deletes all children and clears all the attributes
     *
     * Any memory, associated with children or attributes,
     * is released.
     */
    virtual void clear()
    {
        // Implement in derived classes
    }

    /**
     * Returns parent node of this node.
     *
     * For Document this returns 'this' pointer.
     */
    Node* parent() const
    {
        return m_parent;
    }

    /**
     * Returns document context associated with this node.
     */
    Document* document() const
    {
        return m_document;
    }

    /**
     * Returns the node name.
     */
    virtual const std::string& name() const = 0;

    /**
     * Returns the node namespace.
     */
    virtual std::string nameSpace() const
    {
        return "";
    }

    /**
     * Returns the node tagname (without namespace).
     */
    virtual std::string tagname() const
    {
        return "";
    }

    /**
     * Sets the new name for the node
     * @param name              New node name
     */
    virtual void name(const std::string& name) = 0;

    /**
     * Sets new name for node
     * @param name              New node name
     */
    virtual void name(const char* name) = 0;

    /**
     * Returns the value of the node
     *
     * The meaning of the value depends on the node type.
     * DOM_DOCUMENT and DOM_ELEMENT don't have values
     */
    virtual const String& value() const;

    /**
     * Sets new value to node.
     */
    virtual void value(const String& /*new_value*/)
    {
        // Implement in derived classes
    }

    /**
     * Sets new value to node
     */
    virtual void value(const char* /*new_value*/)
    {
        // Implement in derived classes
    }

    /**
     * Returns cdatas combined from children.
     *
     * E.g. "Some <tag>text</tag> here" becomes: "Some text here"
     */
    String text() const;

    /**
     * Sets text for the node.
     *
     * First, the node child nodes are removed.
     * Then, new NodeText is added to this node.
     * @param txt               Node text
     */
    void text(const String& txt);

    /**
     * Returns referrence to node attributes
     *
     * Returns 0 if node isn't Element or Document
     */
    virtual Attributes& attributes()
    {
        throw Exception("This node can't have attributes");
    }

    /**
     * Returns referrence to node attributes (const version)
     *
     * Returns 0 if node isn't Element or Document
     */
    virtual const Attributes& attributes() const
    {
        throw Exception("This node can't have attributes");
    }

    /**
     * Returns true, if node has any attributes
     */
    virtual bool hasAttributes() const
    {
        return false;
    }

    /**
     * Returns true, if given attribute is found
     * @param attr              Name of attribute
     */
    virtual bool hasAttribute(const char* attr) const
    {
        return false;
    }

    /**
     * Returns attribute value for given attribute.
     *
     * HTML tags can have empty attributes, for those you should use has_attribute() method.
     * @param attr              Name of attribute
     * @param defaultValue      A default value. If attribute doesn't exist then default value is returned.
     * @returns attribute value
     */
    virtual Value getAttribute(const std::string& attr, const char* defaultValue = "") const
    {
        return defaultValue;
    }

    /**
     * Sets new value to attribute 'attr'.
     *
     * If attribute is not found, it's added to map.
     * @param attr              Attribute name
     * @param value             Attribute value
     * @param defaultValue      A default value. If attribute value is matching default value than attribute isn't stored (or removed if it existed).
     */
    virtual void setAttribute(const char* attr, Value value, const char* defaultValue = "")
    {
        // Implement in derived classes
    }

    /**
     * Sets new value to attribute 'attr'.
     *
     * If attribute is not found, it's added to map.
     * @param attr              Attribute name
     * @param value             Attribute value
     * @param defaultValue      A default value. If attribute value is matching default value than attribute isn't stored (or removed if it existed).
     */
    virtual void setAttribute(const std::string& attr, Value value, const char* defaultValue = "")
    {
        // Implement in derived classes
    }

    /**
     * Save node to buffer.
     * @param buffer            Buffer to save to
     * @param indent            Number of indent spaces at start
     */
    virtual void save(Buffer& buffer, int indent = 0) const;

    /**
     * Save node to JSON document
     * @param json              JSON element
     */
    virtual void exportTo(json::Element& json) const;

    /**
     * Returns the first subnode iterator
     */
    virtual iterator begin();

    /**
     * Returns the first subnode const iterator
     */
    virtual const_iterator begin() const;

    /**
     * Returns the end subnode iterator
     */
    virtual iterator end();

    /**
     * Returns the end subnode const iterator
     */
    virtual const_iterator end() const;

    /**
     * Returns a number of subnodes
     */
    virtual uint32_t size() const
    {
        return 0;
    }

    /**
     * Returns true if node has no subnodes of subnodes
     */
    virtual bool empty() const
    {
        return true;
    }

    /**
     * Appends a subnode
     */
    virtual void push_back(Node* node)
    {
        // Implement in derived classes
    }

    /**
     * Inserts a subnode
     *
     * @param pos               Insert position with the list of subnodes
     * @param node              Node to insert
     */
    virtual void insert(iterator pos, Node* node)
    {
        // Implement in derived classes
    }

    /**
     * Removes a subnode
     *
     * Release all the allocated memory and disconnects from parent (this node)
     */
    virtual void remove(Node* node)
    {
        // Implement in derived classes
    }

    /**
     * Removes a subnode
     *
     * Disconnects subnode from parent (this node)
     */
    virtual void unlink(Node* node)
    {
        // Implement in derived classes
    }

    /**
     * Document node, can contain subnodes and attributes
     */
    bool isDocument() const
    {
        return type() == DOM_DOCUMENT;
    }

    /**
     * Normal element node or document, can contain subnodes and attributes
     */
    bool isElement() const
    {
        return (type() & (DOM_ELEMENT | DOM_DOCUMENT)) != 0;
    }

    /**
     * Processing Instruction node
     */
    bool isPI() const
    {
        return type() == DOM_PI;
    }

    /**
     * Cdata where all default entities MUST be escaped.
     */
    bool isText() const
    {
        return type() == DOM_TEXT;
    }

    /**
     * Cdata section, which can contain preformatted char data.
     */
    bool isCDataSection() const
    {
        return type() == DOM_CDATA_SECTION;
    }

    /**
     * Comment node
     */
    bool isComment() const
    {
        return type() == DOM_COMMENT;
    }
};

class SP_EXPORT NodeSearchAlgorithms
{
public:
    static void scanDescendents(Node* thisNode, NodeVector& nodes, const std::vector<XPathElement>& pathElements, int pathPosition,
                                const std::string* starPointer);
    static void matchNode(Node* thisNode, NodeVector& nodes, const std::vector<XPathElement>& pathElements, int pathPosition,
                          const std::string* starPointer);
    static void matchNodesThisLevel(Node* thisNode, NodeVector& nodes, const std::vector<XPathElement>& pathElements, int pathPosition,
                                    const std::string* starPointer, NodeVector& matchedNodes, bool descendants);
    static bool matchPathElement(Node* thisNode, const XPathElement& pathElement, const std::string* starPointer, bool& nameMatches);
};

/**
 * Named item
 *
 * Used as a base class for XML element and XML attribute
 */
class SP_EXPORT NamedItem : public Node
{
    friend class Document;
    friend class Attribute;

    /**
     * Node name, stored in the parent document SST
     */
    const std::string* m_name {nullptr};


protected:
    /**
     * Protected constructor for creating Doc only
     *
     * @param doc a document.
     */
    explicit NamedItem(Document& doc)
    : Node(doc)
    {
    }

    /**
     * Returns true if node name pointer (from SST) matches aname pointer
     * @param sstName           Node name pointer to compare with this node name pointer
     */
    virtual bool nameIs(const std::string* sstName) const
    {
        return sstName == m_name;
    }

public:
    /**
     * Constructor
     *
     * @param parent            Parent node.
     * @param tagname           Name of XML tag
     */
    NamedItem(Node& parent, const char* tagname)
    : Node(parent)
    {
        NamedItem::name(tagname);
    }

    /**
     * Constructor
     *
     * @param parent            Parent node.
     * @param tagname           Name of XML tag
     */
    NamedItem(Node* parent, const char* tagname)
    : Node(*parent)
    {
        NamedItem::name(tagname);
    }

    /**
     * Constructor
     *
     * @param parent            Parent node.
     * @param tagname           Name of XML tag
     */
    NamedItem(Node& parent, const std::string& tagname)
    : Node(parent)
    {
        NamedItem::name(tagname);
    }

    /**
     * Returns the node name.
     */
    virtual const std::string& name() const
    {
        return *m_name;
    }

    /**
     * Returns the node name space.
     */
    virtual std::string nameSpace() const
    {
        size_t pos = m_name->find(":");
        if (pos == std::string::npos)
            return "";
        return m_name->substr(0, pos);
    }

    /**
     * Returns the node tagname without namespace.
     */
    virtual std::string tagname() const
    {
        size_t pos = m_name->find(":");
        if (pos == std::string::npos)
            return *m_name;
        return m_name->substr(pos + 1);
    }

    /**
     * Sets the new name for the node
     * @param name              New node name
     */
    virtual void name(const std::string& name);

    /**
     * Sets new name for node
     * @param name              New node name
     */
    virtual void name(const char* name);

    /**
     * Returns node type
     */
    virtual NodeType type() const
    {
        return DOM_ATTRIBUTE;
    }
};

/**
 * Base class for XML nodes with value
 */
class SP_EXPORT BaseTextNode : public Node
{
    /**
     * Node value
     */
    String m_value;

protected:
    /**
     * returns node name
     */
    virtual const std::string& nodeName() const;

public:
    /**
     * Constructor
     */
    BaseTextNode(Node* parent, const char* data)
    : Node(*parent)
    {
        BaseTextNode::value(data);
    }

    /**
     * Returns the value of the node
     *
     * The meaning of the value depends on the node type.
     * DOM_DOCUMENT and DOM_ELEMENT don't have values
     */
    virtual const String& value() const
    {
        return m_value;
    }

    /**
     * Sets new value to node.
     *
     * @param new_value         New value
     * @see value()
     */
    virtual void value(const String& new_value)
    {
        m_value = new_value;
    }

    /**
     * Sets new value to node
     *
     * @param new_value         New value
     * @see value()
     */
    virtual void value(const char* new_value)
    {
        m_value = new_value;
    }

    /**
     * Returns the node name.
     *
     * The meaning of the value depends on the node type
     */
    virtual const std::string& name() const
    {
        return nodeName();
    }

    /**
     * Sets the new name for the node
     * @param name              New node name
     */
    virtual void name(const std::string& name)
    {
    }

    /**
     * Sets new name for node
     * @param name              New node name
     */
    virtual void name(const char* name)
    {
    }

};

/**
 * XML Text
 */
class SP_EXPORT Text : public BaseTextNode
{
protected:
    /**
     * returns node name
     */
    virtual const std::string& nodeName() const;

public:
    /**
     * Constructor
     *
     * @param parent            Parent node.
     * @param data              Text
     */
    Text(Node& parent, const char* data)
            : BaseTextNode(&parent, data)
    {
    }

    /**
     * Constructor
     *
     * @param parent            Parent node.
     * @param data              Text
     */
    Text(Node* parent, const char* data)
            : BaseTextNode(parent, data)
    {
    }

    /**
     * Constructor
     *
     * @param parent            Parent node.
     * @param data              Text
     */
    Text(Node& parent, const std::string& data)
            : BaseTextNode(&parent, data.c_str())
    {
    }

    /**
     * Returns node type
     */
    virtual NodeType type() const
    {
        return DOM_TEXT;
    }
};

/**
 * XML comment
 */
class SP_EXPORT Comment : public BaseTextNode
{
protected:
    /**
     * returns node name
     */
    virtual const std::string& nodeName() const;

public:
    /**
     * Constructor
     *
     * @param parent            Parent node.
     * @param data              Comment
     */
    Comment(Node& parent, const char* data)
            : BaseTextNode(&parent, data)
    {
    }

    /**
     * Constructor
     *
     * @param parent            Parent node.
     * @param data              Comment
     */
    Comment(Node* parent, const char* data)
            : BaseTextNode(parent, data)
    {
    }

    /**
     * Constructor
     *
     * @param parent            Parent node.
     * @param data              Comment
     */
    Comment(Node& parent, const std::string& data)
            : BaseTextNode(&parent, data.c_str())
    {
    }

    /**
     * Returns node type
     */
    virtual NodeType type() const
    {
        return DOM_COMMENT;
    }
};

/**
 * XML CData section
 */
class SP_EXPORT CDataSection : public BaseTextNode
{
protected:
    /**
     * returns node name
     */
    virtual const std::string& nodeName() const;

public:
    /**
     * Constructor
     *
     * @param parent            Parent node.
     * @param data              Data
     */
    CDataSection(Node& parent, const char* data)
            : BaseTextNode(&parent, data)
    {
    }

    /**
     * Constructor
     *
     * @param parent            Parent node.
     * @param data              Data
     */
    CDataSection(Node* parent, const char* data)
            : BaseTextNode(parent, data)
    {
    }

    /**
     * Constructor
     *
     * @param parent            Parent node.
     * @param data              Data
     */
    CDataSection(Node& parent, const std::string& data)
            : BaseTextNode(&parent, data.c_str())
    {
    }

    /**
     * Returns node type
     */
    virtual NodeType type() const
    {
        return DOM_CDATA_SECTION;
    }
};

/**
 * XML processing instructions (PI)
 */
class SP_EXPORT PI : public BaseTextNode
{
    /**
     * Node name, stored in the parent document SST
     */
    const std::string* m_name {nullptr};

public:
    /**
     * Constructor
     *
     * @param parent            Parent node. Make sure it's a pointer to the existing node.
     * @param target            Target tag name
     * @param data              Data
     */
    PI(Node& parent, std::string target, const char* data)
    : BaseTextNode(&parent, data)
    {
        PI::name(target);
    }

    /**
     * Constructor
     *
     * @param parent            Parent node. Make sure it's a pointer to the existing node.
     * @param target            Target tag name
     * @param data              Data
     */
    PI(Node* parent, std::string target, const char* data)
    : BaseTextNode(parent, data)
    {
        PI::name(target);
    }

    /**
     * Constructor
     *
     * @param parent            Parent node
     * @param target            Target tag name
     * @param data              Data
     */
    PI(Node& parent, std::string target, const std::string& data)
    : BaseTextNode(&parent, data.c_str())
    {
        PI::name(target);
    }

    /**
     * Returns the node name.
     *
     * The meaning of the value depends on the node type
     */
    virtual const std::string& name() const
    {
        return *m_name;
    }

    /**
     * Sets the new name for the node
     * @param name              New node name
     */
    virtual void name(const std::string& name);

    /**
     * Sets new name for node
     * @param name              New node name
     */
    virtual void name(const char* name);

    /**
     * Returns node type
     */
    virtual NodeType type() const
    {
        return DOM_PI;
    }
};

/**
 * @}
 */
}

#define XML_TYPE_DOC             xml::Node::DOM_DOCUMENT         ///< Node type is a document
#define XML_TYPE_ELEMENT         xml::Node::DOM_ELEMENT          ///< Node type is an element
#define XML_TYPE_PI              xml::Node::DOM_PI               ///< Node type is a processing instruction
#define XML_TYPE_TEXT            xml::Node::DOM_TEXT             ///< Node type is a text
#define XML_TYPE_CDATA_SECTION   xml::Node::DOM_CDATA_SECTION    ///< Node type is CDataSection
#define XML_TYPE_COMMENT         xml::Node::DOM_COMMENT          ///< Node type is a comment

}
#endif
