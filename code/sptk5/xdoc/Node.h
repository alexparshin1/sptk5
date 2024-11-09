/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2024 Alexey Parshin. All rights reserved.       ║
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
#include <sptk5/xdoc/NodeName.h>

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
    : public NodeName
    , public std::enable_shared_from_this<Node>
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

    Node(const NodeName& nodeName = "", Type type = Type::Null);

    virtual ~Node() = default;

    virtual void clear();

    virtual void clearChildren();

    Type type() const
    {
        return m_type;
    }

    void type(Type type)
    {
        m_type = type;
    }

    SNode pushNode(const NodeName& name, Type type = Type::Null);

    /**
     * @brief   Push named property to object
     * @details If the value type isn't provided and value isn't null, the type deducted from value.
     * @param name              Property name
     * @param value             Property value
     * @param type              Optional type
     * @return created node
     */
    SNode pushValue(const NodeName& name, const Variant& value, Node::Type type = Node::Type::Null);

    /**
     * @brief   Push value to array
     * @details If the value type isn't provided and value isn't null, the type deducted from value.
     * @param value             Property value
     * @param type              Optional type
     * @return created node
     */
    SNode pushValue(const Variant& value, Node::Type type = Node::Type::Null);

    /**
     * @brief Get node attributes
     * @return node attributes
     */
    Attributes& attributes();

    /**
     * @brief Get node attributes
     * @return node attributes
     */
    const Attributes& attributes() const;

    /**
     * @brief Get node value as string
     * @param name             Optional node name
     * @return node value
     */
    String getString(const NodeName& name = "") const;

    /**
     * @brief Get node value as text
     * @param name             Optional node name
     * @return node value
     */
    String getText(const NodeName& name = "") const;

    /**
     * @brief Get node value as number
     * @param name             Optional node name
     * @return node value
     */
    double getNumber(const NodeName& name = "") const;

    /**
     * @brief Get node value as boolean
     * @param name             Optional node name
     * @return node value
     */
    bool getBoolean(const NodeName& name = "") const;

    /**
     * @brief Get child nodes
     * @param name             Optional node name
     * @return child nodes
     */
    const Nodes& nodes(const NodeName& name = "") const;

    /**
     * @brief Get node value
     * @return node value
     */
    const Variant& getValue() const
    {
        return m_value;
    }

    /**
     * @brief Set node value
     * @tparam T                Data type
     * @param value             Node value
     */
    template<typename T>
    void set(const T& value)
    {
        m_value = value;
    }

    /**
     * @brief Set node value
     * @tparam T                Data type
     * @param name              Node name
     * @param value             Node value
     * @return node
     */
    template<typename T>
    SNode set(const NodeName& name, const T& value)
    {
        auto node = findOrCreate(name);
        node->m_value = value;
        node->type(variantTypeToNodeType(node->m_value.dataType()));

        return node;
    }

    void setNameSpaceRecursive(const String& nameSpace);

    /**
     * @brief Remove node
     * @param name             Node name
     * @return true if node was removed
     */
    bool remove(const NodeName& name);

    /**
     * @brief Remove node
     * @param node              Node to remove
     * @return true if node was removed
     */
    bool remove(const SNode& node);

    /**
     * @brief Find existing node or create a new one
     * @param name              Node name
     * @return node
     */
    SNode findOrCreate(const NodeName& name);

    /**
     * Find first node matching name
     * @param name              Node name to match
     * @param searchMode        Search mode
     * @return
     */
    SNode findFirst(const NodeName& name, SearchMode searchMode = SearchMode::Recursive) const;

    /**
     * @brief Get parent node
     * @return
     */
    SNode parent() const
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
    SNode      m_parent {nullptr};
    Type       m_type {Type::Null};
    Variant    m_value;
    Attributes m_attributes;
    Nodes      m_nodes;

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
