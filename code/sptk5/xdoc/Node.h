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

#include <sptk5/xdoc/Attributes.h>
#include <sptk5/Variant.h>
#include <list>

namespace sptk::xdoc {

enum class DataFormat
    : uint8_t
{
    JSON,
    XML
};

class Node
    : public Variant
{
public:

    using Nodes = std::vector<Node>;
    using Vector = std::vector<Node*>;
    using iterator = Nodes::iterator;
    using const_iterator = Nodes::const_iterator;

    enum class SearchMode
        : uint8_t
    {
        ImmediateChild,
        Recursive
    };

    enum class Type
        : uint8_t
    {
        DocumentRoot,
        Null,
        Text,
        Number,
        Boolean,
        Array,
        Object,
        CData,
        Comment,
        ProcessingInstruction
    };

    Node(const String& nodeName = "", Type type = Type::Null);

    ~Node() override = default;

    virtual void clear();

    virtual void clearChildren();

    String name() const
    {
        return m_name;
    }

    void name(const String& name)
    {
        m_name = name;
    }

    bool is(Type type) const
    {
        return m_type == type;
    }

    Type type() const
    {
        return m_type;
    }

    void type(Type type)
    {
        m_type = type;
    }

    Node& pushNode(const String& name, Type type = Type::Null);

    template<typename T>
    Node& pushValue(const String& name, Type type, const T& value)
    {
        Variant v(value);
        auto& node = pushNode(name, type);
        node.setData(v);
        return node;
    }

    bool empty() const
    {
        return m_nodes.empty();
    }

    Attributes& attributes();

    const Attributes& attributes() const;

    bool hasAttribute(const String& name) const;

    String getAttribute(const String& name, const String& defaultValue = "") const;

    void setAttribute(const String& name, const String& value);

    String getString(const String& name = "") const;

    String text(const String& name = "") const;

    double getNumber(const String& name = "") const;

    bool getBoolean(const String& name = "") const;

    const Nodes& getArray(const String& name = "") const;

    const Node& getObject(const String& name = "") const;

    // Compatibility
    size_t size() const;

    Node& add_array(const String& name);

    Node& add_object(const String& name);

    Node& push_back(const Variant& value)
    {
        auto& node = pushNode("", Type::Null);
        node.setData(value);
        return node;
    }

    Node& push_object();

    Node& set(const String& name, const Variant& value)
    {
        auto& node = findOrCreate(name);
        node.setData(value);
        return node;
    }

    bool remove(const String& name);

    Variant& operator[](const String& name)
    {
        return findOrCreate(name);
    }

    const Variant& operator[](const String& name) const
    {
        auto* pNode = findFirst(name);
        if (pNode == nullptr)
        {
            throw Exception("Element " + name + " doesn't exist");
        }
        return *pNode;
    }

    iterator begin()
    {
        return m_nodes.begin();
    }

    const_iterator begin() const
    {
        return m_nodes.begin();
    }

    iterator end()
    {
        return m_nodes.end();
    }

    const_iterator end() const
    {
        return m_nodes.end();
    }

    Node& findOrCreate(const String& name);

    Node* findFirst(const String& name, SearchMode searchMode = SearchMode::Recursive);

    const Node* findFirst(const String& name, SearchMode searchMode = SearchMode::Recursive) const;

    /**
     * Parse JSON text
     * Root element should have JDT_NULL type (empty element) before calling this method.
     * @param node              Output node
     * @param json              JSON text
     */
    static void importJson(xdoc::Node& node, const sptk::Buffer& json);

    /**
     * Export to JSON text
     * @param node              Output node
     * @param formatted         Format JSON output
     */
    void exportJson(sptk::Buffer& json, bool formatted) const;

    /**
     * Parse XML text
     * Root element should have JDT_NULL type (empty element) before calling this method.
     * @param node              Output node
     * @param xmlKeepSpaces     Import XML mode
     */
    void importXML(const Buffer& xml, bool xmlKeepSpaces = false);

    /**
     * Export to XML text
     * @param node              Output node
     * @param indent            Indent (spaces)
     */
    void exportXML(sptk::Buffer& xml, int indent) const;

    void load(DataFormat dataFormat, const Buffer& data, bool xmlKeepFormatting = false);

    void load(DataFormat dataFormat, const String& data, bool xmlKeepSpaces = false);

    void exportTo(DataFormat dataFormat, Buffer& data, bool formatted) const;

    Node* parent();

    const Node* parent() const;

    virtual void select(Node::Vector& selectedNodes, const String& xpath);

private:

    Node* m_parent {nullptr};
    String m_name;
    Type m_type {Type::Null};
    Attributes m_attributes;
    Nodes m_nodes;

    void exportJsonValueTo(std::ostream& stream, bool formatted, size_t indent) const;

    void exportJsonArray(std::ostream& stream, bool formatted, size_t indent, const String& firstElement,
                         const String& betweenElements, const String& newLineChar, const String& indentSpaces) const;

    void exportJsonObject(std::ostream& stream, bool formatted, size_t indent, const String& firstElement,
                          const String& betweenElements, const String& newLineChar, const String& indentSpaces) const;

};

using Element = Node;

}
