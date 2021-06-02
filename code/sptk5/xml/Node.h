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

#include <sptk5/Buffer.h>
#include <sptk5/Strings.h>
#include <sptk5/DateTime.h>
#include <sptk5/Variant.h>
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
    XPA_CHILD,      ///< Child axis
    XPA_DESCENDANT, ///< Descendant axis
    XPA_PARENT      ///< Parent Axis
};

/**
 * Parsed element of XPath
 */
class SP_EXPORT XPathElement
{
public:
    String      elementName;                   ///< Node name, or '*'
    String      criteria;                      ///< Criteria
    XPathAxis   axis {XPA_CHILD};              ///< Axis
    String      attributeName;                 ///< Attribute name (optional)
    String      attributeValue;                ///< Attribute value (optional)
    bool        attributeValueDefined {false}; ///< true if attribute value was defined
    int         nodePosition {0};              ///< 0 (not required), -1 (last), or node position
};

/**
 * XML Node iterators
 */
class SP_EXPORT NodeIterators
{
public:
    /**
     * xml::Node own iterator for subnodes
     */
    typedef NodeList::iterator iterator;

    /**
     * xml::Node own const_iterator for subnodes
     */
    typedef NodeList::const_iterator const_iterator;

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

private:

    static NodeList emptyNodes;

};

class Node;

/**
 * Base XML Node class
 */
class SP_EXPORT NodeBase: public NodeIterators
{
public:
    /**
     * Node type enumeration
     */
    enum NodeType
    {
        DOM_UNDEFINED = 0,      ///< Type isn't defined yet
        DOM_DOCUMENT = 1,       ///< Document node
        DOM_ELEMENT = 2,        ///< Normal element node, can contain subnodes
        DOM_PI = 4,             ///< Processing Instruction node
        DOM_TEXT = 8,           ///< CDATA where all default entities MUST be escaped
        DOM_CDATA_SECTION = 16, ///< CDATA section, which can contain preformatted char data
        DOM_COMMENT = 32,       ///< Comment node
        DOM_ATTRIBUTE = 64      ///< Attribute node
    };

    /**
     * Constructor
     * @param document          Parent XML document
     */
    explicit NodeBase(Document* document)
    : m_document(document)
    {}

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
     * Always returns false for xml::Node since it has no name
     */
    virtual bool nameIs(const String& /*sstName*/) const
    {
        return false;
    }

    /**
     * Returns the node name.
     */
    virtual String name() const = 0;

    /**
     * Sets the new name for the node
     * @param name              New node name
     */
    virtual void name(const String& name) = 0;

    /**
     * Returns node type
     */
    virtual NodeType type() const = 0;

    /**
     * Returns the node namespace.
     */
    virtual String nameSpace() const
    {
        return "";
    }

    /**
     * Returns the node tagname (without namespace).
     */
    virtual String tagname() const
    {
        return "";
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

protected:

    /**
     * Sets parent node - for XMLParser
     */
    void setParent(Node* p, bool minimal);

private:
    /**
     * Parent document pointer
     */
    Document* m_document;

    /**
     * Parent node pointer
     */
    Node* m_parent {nullptr};
};

/**
 * XML node
 *
 * Basic class for any XML node
 */
class SP_EXPORT Node: public NodeBase
{
    friend class NodeList;
    friend class Document;
    friend class Element;
    friend class Attribute;
    friend class Attributes;

    friend class NodeSearchAlgorithms;

public:

    /**
     * Finds the first subnode with the given name
     *
     * Returns node             Pointer or NULL, if the node with such name is not found.
     * @param name              The name to find
     * @param recursively       If true, also search in all subnodes
     */
    Node* findFirst(const String& name, bool recursively = true) const;

    /**
     * Finds the first subnode with the given name, or creates a new one.
     *
     * Returns node pointer. If the node with such name is not found, then new
     * node is created.
     * @param name              The name to find
     * @param recursively       If true, also search in all subnodes
     */
    Node* findOrCreate(const String& name, bool recursively = true);

    template <class T>
    T* add(const String& nameOrData)
    {
        return new T(this, nameOrData.c_str());
    }

    /**
     * Finds the first subnode with the given name, or creates a new one.
     *
     * Returns node pointer. If the node with such name is not found, then new
     * node is created.
     * @param name              The name to find
     */
    Node& operator[](const String& name)
    {
        return *findOrCreate(name, false);
    }

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
    virtual Variant getAttribute(const String& attr, const char* defaultValue = "") const
    {
        return Variant(defaultValue);
    }

    /**
     * Sets new value to attribute 'attr'.
     *
     * If attribute is not found, it's added to map.
     * @param attr              Attribute name
     * @param value             Attribute value
     * @param defaultValue      A default value. If attribute value is matching default value than attribute isn't stored (or removed if it existed).
     */
    virtual void setAttribute(const String& attr, const Variant& value, const char* defaultValue = "")
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

protected:

    /**
     * Protected constructor - for derived classes
     *
     * @param doc               Node document
     */
    explicit Node(Document& doc)
    : NodeBase(&doc)
    {}

    /**
     * Protected constructor - for derived classes
     *
     * @param parent            Node document
     */
    explicit Node(Node& parent, bool)
    : NodeBase(parent.document())
    {
        parent.push_back(this);
    }

    Node(const Node&) = default;
    Node(Node&&) noexcept = default;
    Node& operator = (const Node&) = default;
    Node& operator = (Node&&) noexcept = default;

    /**
     * Destructor
     */
    virtual ~Node()
    {
    }

private:
    /**
     * Save node to JSON object.
     * @param json              JSON element
     * @param text              Temporary text buffer
     */
    virtual void save(json::Element& json, std::string& text) const;

    void saveElement(const String& nodeName, Buffer& buffer, int indent) const;
    void saveAttributes(Buffer& buffer) const;
    void saveAttributes(json::Element* object) const;
    void saveElement(json::Element* object) const;
    void appendSubNodes(Buffer& buffer, int indent, bool only_cdata) const;
    void appendClosingTag(Buffer& buffer, int indent, bool only_cdata) const;

    void saveTextOrCDATASection(json::Element* object) const;
    void setJsonValue(json::Element* object, const String& nodeText) const;
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
    static void scanDescendents(const Node* thisNode, NodeVector& nodes, const std::vector<XPathElement>& pathElements, int pathPosition,
                                const String& starPointer);

    /**
     * Match nodes
     */
    static void matchNode(Node* thisNode, NodeVector& nodes, const std::vector<XPathElement>& pathElements, int pathPosition,
                          const String& starPointer);

    /**
     * Match nodes only this level
     */
    static void matchNodesThisLevel(const Node* thisNode, NodeVector& nodes, const std::vector<XPathElement>& pathElements, int pathPosition,
                                    const String& starPointer, NodeVector& matchedNodes, bool descendants);

    /**
     * Match path element
     */
    static bool matchPathElement(Node* thisNode, const XPathElement& pathElement,
                                 const String& starPointer);

    static bool matchPathElementAttribute(Node* thisNode, const XPathElement& pathElement, const String& starPointer);
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

public:
    /**
     * Constructor
     *
     * @param parent            Parent node.
     * @param tagname           Name of XML tag
     */
    NamedItem(Node& parent, const char* tagname)
    : Node(parent, true)
    {
        NamedItem::name(tagname);
    }

    NamedItem(const NamedItem&) = default;
    NamedItem(NamedItem&&) noexcept = default;
    NamedItem& operator = (const NamedItem&) = default;
    NamedItem& operator = (NamedItem&&) noexcept = default;

    /**
     * Constructor
     *
     * @param parent            Parent node.
     * @param tagname           Name of XML tag
     */
    NamedItem(Node& parent, const String& tagname)
    : Node(parent, true)
    {
        NamedItem::name(tagname);
    }

    /**
     * Returns the node name.
     */
    String name() const override
    {
        return m_name;
    }

    /**
     * Returns the node name space.
     */
    String nameSpace() const override
    {
        size_t pos = m_name.find(":");
        if (pos == std::string::npos)
            return "";
        return m_name.substr(0, pos);
    }

    /**
     * Returns the node tagname without namespace.
     */
    String tagname() const override
    {
        size_t pos = m_name.find(":");
        if (pos == std::string::npos)
            return m_name;
        return m_name.substr(pos + 1);
    }

    /**
     * Sets the new name for the node
     * @param name              New node name
     */
    void name(const String& name) override;

    /**
     * Returns node type
     */
    NodeType type() const override
    {
        return DOM_ATTRIBUTE;
    }

protected:
    /**
     * Protected constructor for creating Doc only
     *
     * @param doc a document.
     */
    explicit NamedItem(Document& doc) : Node(doc)
    {
    }

    /**
     * Returns true if node name pointer (from SST) matches aname pointer
     * @param name           Node name pointer to compare with this node name pointer
     */
    bool nameIs(const String& name) const override
    {
        return name == m_name;
    }

private:

    String  m_name;    ///< Node name
};

/**
 * Base class for XML nodes with value
 */
class SP_EXPORT BaseTextNode : public Node
{
public:
    /**
     * Constructor
     */
    BaseTextNode(Node* parent, const char* data)
    : Node(*parent, true)
    {
        BaseTextNode::value(data);
    }

    /**
     * Returns the value of the node
     *
     * The meaning of the value depends on the node type.
     * DOM_DOCUMENT and DOM_ELEMENT don't have values
     */
    const String& value() const override
    {
        return m_value;
    }

    /**
     * Sets new value to node.
     *
     * @param new_value         New value
     * @see value()
     */
    void value(const String& new_value) override
    {
        m_value = new_value;
    }

    /**
     * Sets new value to node
     *
     * @param new_value         New value
     * @see value()
     */
    void value(const char* new_value) override
    {
        m_value = new_value;
    }

    /**
     * Returns the node name.
     *
     * The meaning of the value depends on the node type
     */
    String name() const override
    {
        return nodeName();
    }

    /**
     * Sets the new name for the node
     * @param name              New node name
     */
    void name(const String& name) override
    {
        // Text node con't have name
    }

protected:
    /**
     * returns node name
     */
    virtual String nodeName() const;

private:

    String m_value;     ///< Node value
};

/**
 * XML Text
 */
class SP_EXPORT Text : public BaseTextNode
{
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
    Text(Node& parent, const String& data)
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

protected:
    /**
     * returns node name
     */
    virtual String nodeName() const;
};

/**
 * XML comment
 */
class SP_EXPORT Comment : public BaseTextNode
{
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
    Comment(Node& parent, const String& data)
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

protected:
    /**
     * returns node name
     */
    virtual String nodeName() const;
};

/**
 * XML CData section
 */
class SP_EXPORT CDataSection : public BaseTextNode
{
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
    CDataSection(Node& parent, const String& data)
    : BaseTextNode(&parent, data.c_str())
    {
    }

    /**
     * Returns node type
     */
    NodeType type() const override
    {
        return DOM_CDATA_SECTION;
    }

protected:
    /**
     * returns node name
     */
    String nodeName() const override;
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
