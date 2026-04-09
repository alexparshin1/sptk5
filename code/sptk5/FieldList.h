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

#include <sptk5/CaseInsensitiveCompare.h>
#include <sptk5/Field.h>
#include <sptk5/xdoc/Node.h>
#include <vector>

namespace sptk {

/**
 * @addtogroup utility Utility Classes.
 * @{
 */

/**
 * @brief The list of Field objects.
 *
 * Is used in DataSource.
 * Allows accessing data fields by the field name or field index.
 * Provides the streaming output and export to XML.
 */
class SP_EXPORT FieldList
{
public:
    /**
     * @brief Field iterator.
     */
    using iterator = std::vector<SField>::iterator;

    /**
     * @brief Field const iterator.
     */
    using const_iterator = std::vector<SField>::const_iterator;

    /**
     * @brief Default constructor.
     *
     * @param indexed           If you want to have a field index by name added, that speeds up the search of the field by name but increases the occupied memory.
     */
    explicit FieldList(bool indexed);

    /**
     * @brief Copy constructor.
     *
     * @param other             The other field list.
     */
    FieldList(const FieldList& other) = delete;

    /**
     * @brief Move constructor.
     *
     * @param other             The other field list.
     */
    FieldList(FieldList&& other) noexcept = default;

    /**
     * @brief Copy assignment.
     *
     * @param other             The other field list.
     */
    FieldList& operator=(const FieldList& other) = delete;

    /**
     * @brief Copy assignment.
     *
     * @param other             The other field list.
     */
    FieldList& operator=(FieldList&& other) noexcept = default;

    /**
     * @brief Clears the field list.
     */
    void clear();

    /**
     * @brief Returns the number of fields in the list.
     */
    [[nodiscard]] size_t size() const
    {
        return m_list.size();
    }

    /**
     * @brief Begin iterator.
     */
    iterator begin()
    {
        return m_list.begin();
    }

    /**
     * @brief Begin const iterator.
     */
    [[nodiscard]] const_iterator begin() const
    {
        return m_list.begin();
    }

    /**
     * @brief End iterator.
     */
    iterator end()
    {
        return m_list.end();
    }

    /**
     * @brief End const iterator.
     */
    [[nodiscard]] const_iterator end() const
    {
        return m_list.end();
    }

    /**
     * @brief Adds a new field into the list.
     *
     * Creates and returns a new field.
     * @param fname             Field name.
     * @param checkDuplicates   If true, check if the field already exists in the list.
     * @returns new field reference.
     */
    Field& push_back(const String& fname, bool checkDuplicates);

    /**
     * @brief Adds a new field into the list without creating a new copy of the field.
     *
     * This method is useful if you create a new field with the new() operator.
     * You shouldn't delete such fields manually - they would be maintained by FieldList class.
     * @param field               Field name.
     * @returns new field reference.
     */
    Field& push_back(const SField& field);

    /**
     * @brief Finds a field by the field name.
     * @param fname             Field name.
     * @returns Field pointer, or null if not found.
     */
    [[nodiscard]] SField findField(const String& fname) const;

    /**
     * @brief Finds a field by the field name.
     * @param fname             Field name.
     * @returns Field pointer, or throw exception not found.
     */
    [[nodiscard]] SField fieldByName(const String& fname) const
    {
        auto field = findField(fname);
        if (!field)
        {
            throw Exception("Field name '" + String(fname) + "' not found");
        }
        return field;
    }

    /**
     * @brief Field access by field index, non-const version.
     *
     * @param index             Field index.
     * @returns field reference.
     */
    Field& operator[](int index)
    {
        if (index < 0 || index >= static_cast<int>(m_list.size()))
        {
            throw Exception("Field index out of range");
        }
        return *m_list[index];
    }

    /**
     * @brief Field access by field index, const version.
     * @param index             Field index.
     * @returns field reference.
     */
    const Field& operator[](int index) const
    {
        if (index < 0 || index >= static_cast<int>(m_list.size()))
        {
            throw Exception("Field index out of range");
        }
        return *m_list[index];
    }

    /**
     * @brief Field access by field name, non-const version.
     * @param fname             Field name.
     * @returns field reference.
     */
    Field& operator[](const String& fname)
    {
        return *fieldByName(fname);
    }

    /**
     * @brief Field access by field name, const version.
     * @param fname             Field name.
     * @returns field reference.
     */
    const Field& operator[](const String& fname) const
    {
        return *fieldByName(fname);
    }

    /**
     * @brief Exports data into an XDoc node.
     *
     * The compact XML modes parameter means that the field values are stored as attributes, w/o type information.
     * Otherwise, fields are stored as subnodes, with the field information stored as attributes.
     * @param node              XDoc node to store fields into.
     * @param compactMode       If true produces compact XML.
     * @param nullLargeData     Set text fields to null if the data length is 256 bytes or longer.
     */
    void exportTo(const xdoc::SNode& node, bool compactMode, bool nullLargeData = false) const;

private:
    /**
     * @brief Field vector.
     */
    using Vector = std::vector<SField>;

    /**
     * @brief Field name to the field object case-insensitive map.
     */
    using Map = std::unordered_map<std::string, SField, std::hash<std::string>, CaseInsensitiveCompare<std::string>>;

    Vector               m_list;  ///< The list of fields
    std::shared_ptr<Map> m_index; ///< The optional field index by name, or nullptr if the field list isn't indexed.
};

} // namespace sptk

/**
 * @}
 */
