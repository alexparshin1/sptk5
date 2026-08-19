/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#pragma once

#include <sptk5/xdoc/Node.h>

namespace sptk::xdoc {

/**
 * @class Document
 *
 * @brief Represents a JSON/XML document.
 * @note This class does not handle concurrent modifications.
 */
class SP_EXPORT Document
{
public:
    /**
     * @brief Constructs a Document with the specified root type.
     * @param rootType The type of the root node (default: Object).
     */
    explicit Document(Node::Type rootType = Node::Type::Object);

    /**
     * @brief Copy constructor.
     * @param other Another Document instance to copy.
     */
    Document(const Document& other);

    /**
     * @brief Move constructor.
     * @param other Another object.
     */
    Document(Document&& other) noexcept;

    /**
     * @brief Copy assignment operator.
     * @param other Another object.
     * @return this document.
     */
    Document& operator=(const Document& other);

    /**
     * @brief Move assignment operator.
     * @param other Another object.
     * @return this document.
     */
    Document& operator=(Document&& other) noexcept;

    /**
     * @brief Destructor.
     */
    ~Document() = default;

    /**
     * @brief Clear the document.
     */
    void clear()
    {
        m_root->clear();
    }

    /**
     * @brief Get the root node of the document.
     * @return root node.
     */
    [[nodiscard]] SNode root() const
    {
        return m_root;
    }

    /**
     * @brief Load a document from the buffer.
     * @param data Buffer containing XML or JSON document data.
     * @param xmlKeepFormatting Whether to keep formatting when loading XML.
     */
    void load(const Buffer& data, bool xmlKeepFormatting = false);

    /**
     * @brief Load a document from the string.
     * @param data Buffer containing XML or JSON document data.
     * @param xmlKeepFormatting Whether to keep formatting when loading XML.
     */
    void load(const String& data, bool xmlKeepFormatting = false);

    /**
     * @brief Export the document to the buffer.
     * @param dataFormat Format to export the document to.
     * @param data Buffer to store the exported document data.
     * @param formatted Whether to format the exported document.
     */
    void exportTo(DataFormat dataFormat, Buffer& data, bool formatted = false) const
    {
        m_root->exportTo(dataFormat, data, formatted);
    }

    /**
     * @brief Export the document to the stream.
     * @param dataFormat Format to export the document to.
     * @param data Stream to write the exported document data to.
     * @param formatted Whether to format the exported document.
     */
    void exportTo(DataFormat dataFormat, std::ostream& data, bool formatted = false) const
    {
        m_root->exportTo(dataFormat, data, formatted);
    }

    /**
     * @brief Find or create a node with the given name.
     * @param name Name of the node to find or create.
     * @return Shared pointer to the found or created node.
     */
    [[nodiscard]] SNode findOrCreate(const String& name) const
    {
        return m_root->findOrCreate(name);
    }

    /**
     * @brief Find the first node with the given name.
     * @param name Name of the node to find.
     * @param searchMode Search mode to use.
     * @return the shared pointer to the found node.
     */
    [[nodiscard]] SNode findFirst(const String& name, SearchMode searchMode = SearchMode::Recursive) const
    {
        return m_root->findFirst(name, searchMode);
    }

    /**
     * @brief Select nodes matching the given XPath.
     * @param xpath XPath to match the nodes.
     * @return matched nodes.
     */
    [[nodiscard]] Node::Vector select(const String& xpath) const
    {
        return m_root->select(xpath);
    }

private:
    SNode m_root; ///< The root node of the document.
};

using SDocument = std::shared_ptr<Document>;

} // namespace sptk::xdoc
