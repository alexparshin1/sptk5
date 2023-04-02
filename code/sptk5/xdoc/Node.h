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

#pragma once

#include <sptk5/Variant.h>
#include <sptk5/xdoc/Attributes.h>

namespace sptk::xdoc {

enum class DataFormat : uint8_t
{
    JSON,
    XML
};

enum class SearchMode : uint8_t
{
    ImmediateChild,
    Recursive
};

class SP_EXPORT Node
    : public std::enable_shared_from_this<Node>
{
public:
    using SNode = std::shared_ptr<Node>;
    using Nodes = std::vector<SNode>;
    using Vector = std::vector<SNode>;
    using iterator = Nodes::iterator;
    using const_iterator = Nodes::const_iterator;

    enum class Type : uint8_t
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

    Node(String nodeName = "", Type type = Type::Null);

    virtual ~Node() = default;

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

    Type type() const
    {
        return m_type;
    }

    void type(Type type)
    {
        m_type = type;
    }

    SNode& pushNode(const String& name, Type type = Type::Null);

    /**
     * @brief   Push named property to object
     * @details If the value type isn't provided and value isn't null, the type deducted from value.
     * @param name              Property name
     * @param value             Property value
     * @param type              Optional type
     * @return created node
     */
    SNode& pushValue(const String& name, const Variant& value, Node::Type type = Node::Type::Null);

    /**
     * @brief   Push value to array
     * @details If the value type isn't provided and value isn't null, the type deducted from value.
     * @param value             Property value
     * @param type              Optional type
     * @return created node
     */
    SNode& pushValue(const Variant& value, Node::Type type = Node::Type::Null);

    Attributes& attributes();

    const Attributes& attributes() const;

    String getString(const String& name = "") const;

    String getText(const String& name = "") const;

    double getNumber(const String& name = "") const;

    bool getBoolean(const String& name = "") const;

    const Nodes& nodes(const String& name = "") const;

    const Variant& getValue() const
    {
        return m_value;
    }

    template<typename T>
    void set(const T& value)
    {
        m_value = value;
    }

    template<typename T>
    SNode& set(const String& name, const T& value)
    {
        auto& node = findOrCreate(name);
        node->m_value = value;
        node->type(variantTypeToNodeType(node->m_value.dataType()));

        return node;
    }

    bool remove(const String& name);

    bool remove(const SNode& node);

    SNode& findOrCreate(const String& name);

    SNode findFirst(const String& name, SearchMode searchMode = SearchMode::Recursive) const;

    SNode& parent()
    {
        return m_parent;
    }

    const SNode& parent() const
    {
        return m_parent;
    }

    /**
     * Parse JSON text
     * Root element should have JDT_NULL type (empty element) before calling this method.
     * @param jsonElement              Output node
     * @param jsonStr              JSON text
     */
    static void importJson(const SNode& jsonElement, const sptk::Buffer& jsonStr);

    void load(DataFormat dataFormat, const Buffer& data, bool xmlKeepFormatting = false);

    void load(DataFormat dataFormat, const String& data, bool xmlKeepFormatting = false);

    void exportTo(DataFormat dataFormat, Buffer& data, bool formatted) const;

    void exportTo(DataFormat dataFormat, std::ostream& stream, bool formatted) const;

    /**
     * @brief Select a list of sub-nodes matching xpath
     * @param xpath             XPath
     * @return                  List of matching sub-nodes
     */
    [[nodiscard]] Node::Vector select(const String& xpath);

    /**
     * @brief Perform a deep copy of the source to destination
     * @param destination       Destination node
     * @param source            Source node
     */
    static void clone(const SNode& destination, const SNode& source);

private:
    SNode m_parent {nullptr};
    String m_name;
    Type m_type {Type::Null};
    Variant m_value;
    Attributes m_attributes;
    Nodes m_nodes;

    static Type variantTypeToNodeType(VariantDataType type);
};

using Element = Node;
using SNode = Node::SNode;

/**
 * Does string match a float?
 * @return true if string constains a float
 */
SP_EXPORT bool isFloat(const String& str);

/**
 * Does string match an integer?
 * @return true if string constains an integer
 */
SP_EXPORT bool isInteger(const String& str);

/**
 * Does string match a boolen?
 * @return true if string constains a boolean
 */
SP_EXPORT bool isBoolean(const String& str);

} // namespace sptk::xdoc
