/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       FieldList.h - description                              ║
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

#ifndef __SPTK_FIELDLIST_H__
#define __SPTK_FIELDLIST_H__

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
    typedef std::vector<Field*>::iterator                              iterator;

    /**
     * Field const iterator
     */
    typedef std::vector<Field*>::const_iterator                        const_iterator;


private:
    /**
     * Field vector
     */
    typedef std::vector<Field*>                                        Vector;

    /**
     * Field name to field case-insensitive map
     */
    typedef std::map<std::string, Field *, CaseInsensitiveCompare>     Map;


    /**
     * User data - any data you want to associate with that field list
     */
    void*                   m_userData;

    /**
     * The list of fields
     */
    Vector                  m_list;

    /**
     * The optional field index by name. 0L if field list isn't indexed.
     */
    Map*                    m_index;

    /**
     * The compact XML mode flag
     */
    bool                    m_compactXmlMode;


public:

    /**
     * Default constructor
     *
     * @param indexed           If you want to have a field index by name added. Such index speeds up the search of the field by name, but increases the occupied memory.
     * @param compactXmlMode    Compact XML export flag, @see xmlMode for details
     */
    FieldList(bool indexed, bool compactXmlMode=true);

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
     * Clears the field list
     */
    void clear();

    /**
     * Returns the nummber of fields in the list
     */
    uint32_t size() const
    {
        return (uint32_t) m_list.size();
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
     * Defines XML export mode
     *
     * The compact XML modes means that fields values are stored as attributes, w/o type information.
     * Otherwise, fields are stored as subnodes, with the field information stored as attributes.
     * @param compact           Compact XML export flag
     */
    void xmlMode(bool compact)
    {
        m_compactXmlMode = compact;
    }

    /**
     * Adds a new field int the list
     *
     * Creates and returns a new field.
     * @param fname             Field name
     * @param checkDuplicates   If true check if the field already exists in the list
     * @returns new field reference
     */
    Field& push_back(const char *fname,bool checkDuplicates);

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
    Field* fieldByName(const char * fname) const;

    /**
     * Field access by field index, non-const version
     *
     * @param index             Field index
     * @returns field reference
     */
    Field& operator [](uint32_t index)
    {
        return *(Field *) m_list[index];
    }

    /**
     * Field access by field index, non-const version
     *
     * @param index             Field index
     * @returns field reference
     */
    Field& operator [](int32_t index)
    {
        return *(Field *) m_list[size_t(index)];
    }

    /**
     * Field access by field index, const version
     * @param index             Field index
     * @returns field reference
     */
    const Field& operator [](uint32_t index) const
    {
        return *(Field *) m_list[size_t(index)];
    }

    /**
     * Field access by field index, const version
     * @param index             Field index
     * @returns field reference
     */
    const Field& operator [](int32_t index) const
    {
        return *(Field *) m_list[size_t(index)];
    }

    /**
     * Field access by field name, non-const version
     * @param fname             Field name
     * @returns field reference
     */
    Field& operator [](const char *fname)
    {
        return *fieldByName(fname);
    }

    /**
     * Field access by field name, const version
     * @param fname             Field name
     * @returns field reference
     */
    const Field& operator [](const char *fname) const
    {
        return *fieldByName(fname);
    }

    /**
     * Field access by field name, non-const version
     * @param fname             Field name
     * @returns field reference
     */
    Field& operator [](const std::string& fname)
    {
        return *fieldByName(fname.c_str());
    }

    /**
     * Field access by field name, const version
     * @param fname             Field name
     * @returns field reference
     */
    const Field& operator [](const std::string& fname) const
    {
        return *fieldByName(fname.c_str());
    }

    /**
     * Sets user data
     *
     * User data is usually a pointer to some outside memory object,
     * or an index (id) of some object. CFieldList doesn't maintain this pointer, just keeps it
     * as a tag.
     * @param data              User-defined data
     */
    void user_data(void *data)
    {
        m_userData = data;
    }

    /**
     * Returns user data
     */
    void* user_data() const
    {
        return m_userData;
    }

    /**
     * Exports data into XML node
     *
     * @see setXmlMode() for details.
     * @param xml               XML node to store fields into
     */
    void toXML(xml::Node& xml) const;
};
}

/**
 * @}
 */

#endif
