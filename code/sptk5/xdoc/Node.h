/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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
    using WNode = std::weak_ptr<Node>;
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

    /**
     * @brief Node factory.
     * @param name              Node name.
     * @param type              Node type.
     * @return created node.
     */
    static SNode createNode(const NodeName& name, Type type = Type::Null);

    /**
     * @brief Node factory.
     * @param other             Another object.
     * @return node.
     */
    static SNode createNode(const SNode& other);

    /**
     * @brief Destructor.
     */
    virtual ~Node() = default;

    /**
     * @brief Clear node.
     * @remarks Removes all children and attributes.
     */
    virtual void clear();

    /**
     * @brief Clear node children.
     */
    virtual void clearChildren();

    /**
     * @brief Get node type.
     * @return node type.
     */
    Type type() const
    {
        return m_type;
    }

    /**
     * @brief Set the node type.
     * @param type Node type.
     */
    void type(const Type type)
    {
        m_type = type;
    }

    /**
     * @brief Push the named node to the object.
     * @param name              Node name.
     * @param type              Node type.
     * @return created node.
     */
    SNode pushNode(const NodeName& name, Type type = Type::Null);

    /**
     * @brief Push the node to arrray.
     * @param name              Node name.
     * @param type              Node type.
     * @return created node.
     */
    SNode pushNode(const SNode& node);

    /**
     * @brief   Push the named property to the object.
     * @details If the value type isn't provided and value isn't null, the type deducted from value.
     * @param name              Property name.
     * @param value             Property value.
     * @param type              Optional type.
     * @return created node.
     */
    SNode pushValue(const NodeName& name, const Variant& value, Type type = Type::Null);

    /**
     * @brief   Push value to array.
     * @details If the value type isn't provided and value isn't null, the type deducted from value.
     * @param value             Property value.
     * @param type              Optional type.
     * @return created node.
     */
    SNode pushValue(const Variant& value, Type type = Type::Null);

    /**
     * @brief Get node attributes.
     * @return node attributes.
     */
    Attributes& attributes();

    /**
     * @brief Get node attributes.
     * @return node attributes.
     */
    const Attributes& attributes() const;

    /**
     * @brief Get node value as string.
     * @param name             Optional node name.
     * @return node value.
     */
    String getString(const NodeName& name = "") const;

    /**
     * @brief Get node value as text.
     * @param name             Optional node name.
     * @return node value.
     */
    String getText(const NodeName& name = "") const;

    /**
     * @brief Get node value as a number.
     * @param name             Optional node name.
     * @return node value.
     */
    double getNumber(const NodeName& name = "") const;

    /**
     * @brief Get node value as boolean.
     * @param name             Optional node name.
     * @return node value.
     */
    bool getBoolean(const NodeName& name = "") const;

    /**
     * @brief Get child nodes.
     * @param name             Optional node name.
     * @return child nodes.
     */
    const Nodes& nodes(const NodeName& name = "") const;

    /**
     * @brief Get node value.
     * @return node value.
     */
    const Variant& getValue() const
    {
        return m_value;
    }

    /**
     * @brief Set node value.
     * @tparam T                Data type.
     * @param value             Node value.
     */
    template<typename T>
    void set(const T& value)
    {
        m_value = value;
        type(variantTypeToNodeType(m_value.dataType()));
    }

    /**
     * @brief Set node value.
     * @tparam T                Data type.
     * @param name              Node name.
     * @param value             Node value.
     * @return node.
     */
    template<typename T>
    SNode set(const NodeName& name, const T& value)
    {
        auto node = findOrCreate(name);
        node->m_value = value;
        node->type(variantTypeToNodeType(node->m_value.dataType()));

        return node;
    }

    void setNamespaceRecursive(const String& nameSpace);

    /**
     * @brief Remove node.
     * @param name             Node name.
     * @return true if the node was removed.
     */
    bool remove(const NodeName& name);

    /**
     * @brief Remove node.
     * @param node              Node to remove.
     * @return true if the node was removed.
     */
    bool remove(const SNode& node);

    /**
     * @brief Find an existing node or create a new one.
     * @param name              Node name.
     * @return node.
     */
    SNode findOrCreate(const NodeName& name);

    /**
     * @brief Find the first node matching name.
     * @param name              Node name to match.
     * @param searchMode        Search mode.
     * @return.
     */
    SNode findFirst(const NodeName& name, SearchMode searchMode = SearchMode::Recursive) const;

    /**
     * @brief Get parent node.
     * @return.
     */
    SNode parent() const
    {
        return m_parent.lock();
    }

    /**
     * @brief Parse JSON text.
     * The root element should have the Type::Null type (empty element) before calling this method.
     * @param jsonElement          Output node.
     * @param jsonStr              JSON text.
     */
    static void importJson(const SNode& jsonElement, const Buffer& jsonStr);

    /**
     * @brief Load data from the buffer.
     * @param dataFormat            Data format, XML or JSON.
     * @param data                  Buffer to load data from.
     * @param xmlKeepFormatting     Flag: keep XML formatting.
     */
    void load(DataFormat dataFormat, const Buffer& data, bool xmlKeepFormatting = false);

    /**
     * @brief Load data from the string.
     * @param dataFormat            Data format, XML or JSON.
     * @param data                  String to load data from.
     * @param xmlKeepFormatting     Flag: keep XML formatting.
     */
    void load(DataFormat dataFormat, const String& data, bool xmlKeepFormatting = false);

    /**
     * @brief Export data to buffer.
     * @param dataFormat            Data format, XML or JSON.
     * @param data                  Buffer to export data to.
     * @param formatted             Flag: pretty-format data.
     */
    void exportTo(DataFormat dataFormat, Buffer& data, bool formatted) const;

    /**
     * @brief Export data to stream.
     * @param dataFormat            Data format, XML or JSON.
     * @param stream                  Stream to export data to.
     * @param formatted             Flag: pretty-format data.
     */
    void exportTo(DataFormat dataFormat, std::ostream& stream, bool formatted) const;

    /**
     * @brief Select a list of sub-nodes matching xpath.
     * @param xpath             XPath.
     * @return                  List of matching sub-nodes.
     */
    [[nodiscard]] Vector select(const String& xpath);

private:
    WNode      m_parent;            ///< Parent node.
    Type       m_type {Type::Null}; ///< Node type.
    Variant    m_value;             ///< Node value.
    Attributes m_attributes;        ///< Node attributes.
    Nodes      m_nodes;             ///< Node child nodes.

    /**
     * @brief Private constructor. Use Node::create() to create nodes.
     * @param nodeName          Node name.
     * @param type              Node type.
     */
    explicit Node(const NodeName& nodeName, Type type);

    static constexpr Type variantTypeToNodeType(VariantDataType const type) noexcept
    {
        using enum Type;
        switch (type)
        {
            using enum VariantDataType;
            case VAR_NONE:
                return Null;

            case VAR_INT:
            case VAR_FLOAT:
            case VAR_IMAGE_NDX:
            case VAR_INT64:
                return Number;

            case VAR_MONEY:
            case VAR_STRING:
            case VAR_TEXT:
            case VAR_BUFFER:
            case VAR_DATE:
            case VAR_DATE_TIME:
            case VAR_IMAGE_PTR:
                return Text;

            case VAR_BOOL:
                return Boolean;

            default:
                break;
        }

        return Null;
    }
};

using Element = Node;
using SNode = Node::SNode;

/**
 * @brief Does string match a float?.
 * @return true if string constains a float.
 */
SP_EXPORT bool isFloat(const String& str);

/**
 * @brief Does the string match an integer?.
 * @return true if string constains an integer.
 */
SP_EXPORT bool isInteger(const String& str);

/**
 * @brief Does the string match a boolean?.
 * @return true if string constains a boolean.
 */
SP_EXPORT bool isBoolean(const String& str);

} // namespace sptk::xdoc
