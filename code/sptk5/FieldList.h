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

#include <sptk5/Field.h>
#include <sptk5/cxml>
#include <sptk5/CaseInsensitiveCompare.h>
#include <map>
#include <vector>

namespace sptk {

/**
 * @addtogroup utility Utility Classes
 * @{
 */

/**
 * The list of CField objects.
 *
 * Is used in CDataSource.
 * Allows to access data fields by the field name or field index.
 * Provides the streaming output, and export to XML.
 */
class SP_EXPORT FieldList
{
public:
    /**
     * Field iterator
     */
    using iterator = std::vector<Field*>::iterator;

    /**
     * Field const iterator
     */
    using const_iterator = std::vector<Field*>::const_iterator;


    /**
     * Default constructor
     *
     * @param indexed           If you want to have a field index by name added. Such index speeds up the search of the field by name, but increases the occupied memory.
     * @param compactXmlMode    Compact XML export flag, @see xmlMode for details
     */
    explicit FieldList(bool indexed);

    /**
     * Copy constructor
     *
     * @param other             Other field list
     */
    FieldList(const FieldList& other);

    /**
     * Destructor
     */
    ~FieldList();

    /**
     * Copy assignment
     *
     * @param other             Other field list
     */
    FieldList& operator = (const FieldList& other);

    /**
     * Clears the field list
     */
    void clear();

    /**
     * Returns the nummber of fields in the list
     */
    size_t size() const
    {
        return m_list.size();
    }

    /**
     * Begin iterator
     */
    iterator begin()
    {
        return m_list.begin();
    }

    /**
     * Begin const iterator
     */
    const_iterator begin() const
    {
        return m_list.begin();
    }

    /**
     * End iterator
     */
    iterator end()
    {
        return m_list.end();
    }

    /**
     * End const iterator
     */
    const_iterator end() const
    {
        return m_list.end();
    }

    /**
     * Adds a new field int the list
     *
     * Creates and returns a new field.
     * @param fname             Field name
     * @param checkDuplicates   If true check if the field already exists in the list
     * @returns new field reference
     */
    Field& push_back(const String& fname, bool checkDuplicates);

    /**
     * Adds a new field int the list without creating a new copy of the field.
     *
     * This method is useful if you create a new field with the new() operator.
     * You shouldn't delete such fields manually - they would be maintained by CFieldList class.
     * @param fld               Field name
     * @returns new field reference
     */
    Field& push_back(Field *fld);

    /**
     * Finds a field by the field name
     *
     * Fast field lookup using std::map.
     * @param fname             Field name
     * @returns CField pointer, or 0L if not found
     */
    Field* findField(const String& fname) const;

    /**
     * Finds a field by the field name
     *
     * Fast field lookup using std::map.
     * @param fname             Field name
     * @returns CField pointer, or throw exception not found
     */
    Field* fieldByName(const String& fname) const
    {
        Field* field = findField(fname);
        if (field == nullptr)
            throw Exception("Field name '" + String(fname) + "' not found");
        return field;
    }

    /**
     * Field access by field index, non-const version
     *
     * @param index             Field index
     * @returns field reference
     */
    Field& operator [](int index)
    {
        return *m_list[index];
    }

    /**
     * Field access by field index, const version
     *
     * @param index             Field index
     * @returns field reference
     */
    const Field& operator [](int index) const
    {
        return *m_list[index];
    }

    /**
     * Field access by field name, non-const version
     * @param fname             Field name
     * @returns field reference
     */
    Field& operator [](const String& fname)
    {
        return *fieldByName(fname);
    }

    /**
     * Field access by field name, const version
     * @param fname             Field name
     * @returns field reference
     */
    const Field& operator [](const String& fname) const
    {
        return *fieldByName(fname.c_str());
    }

    /**
     * Exports data into XML node
     *
     * The compact XML modes means that fields values are stored as attributes, w/o type information.
     * Otherwise, fields are stored as subnodes, with the field information stored as attributes.
     * @param node              XML node to store fields into
     * @param compact           Compact XML export flag
     */
    void toXML(xml::Node& node, bool compactMode=false) const;

private:
    /**
     * Field vector
     */
    using Vector = std::vector<Field*>;

    /**
     * Field name to field case-insensitive map
     */
    using Map = std::map<String, Field *, CaseInsensitiveCompare>;

    Vector                  m_list;                     ///< The list of fields
    std::shared_ptr<Map>    m_index;                    ///< The optional field index by name. 0L if field list isn't indexed.

    /**
     * Copy assignment
     *
     * @param other             Other field list
     */
    void assign(const FieldList& other);};
}

/**
 * @}
 */

