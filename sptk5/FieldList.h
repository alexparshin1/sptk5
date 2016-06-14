/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       FieldList.h - description                              ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
║  copyright            (C) 1999-2016 by Alexey Parshin. All rights reserved.  ║
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

/// @addtogroup utility Utility Classes
/// @{

/// @brief The list of CField objects.
///
/// Is used in CDataSource.
/// Allows to access data fields by the field name or field index.
/// Provides the streaming output, and export to XML.
class SP_EXPORT FieldList
{
    typedef std::map<std::string, Field *, CaseInsensitiveCompare>     CFieldMap;
    typedef std::vector<Field *>                                       CFieldVector;

    void*                   m_userData;        ///< User data - any data you want to associate with that field list
    CFieldVector            m_list;            ///< The list of fields
    CFieldVector::iterator  m_fieldStreamItor; ///< The field iterator for the streamed fields reading
    CFieldMap*              m_index;           ///< The optional field index by name. 0L if field list isn't indexed.
    bool                    m_compactXmlMode;  ///< The compact XML mode flag

public:

    /// @brief Default constructor
    ///
    /// @param indexed bool, if you want to have a field index by name added. Such index speeds up the search of the field by name, but increases the occupied memory.
    /// @param compactXmlMode bool, the compact XML export flag, @see xmlMode for details
    FieldList(bool indexed, bool compactXmlMode=true);

    /// Destructor
    ~FieldList();

    /// Clears the field list
    void clear();

    /// Returns the nummber of fields in the list
    uint32_t size() const
    {
        return (uint32_t) m_list.size();
    }

    /// @brief Defines XML export mode
    ///
    /// The compact XML modes means that fields values are stored as attributes, w/o type information.
    /// Otherwise, fields are stored as subnodes, with the field information stored as attributes.
    /// @param compact bool, the compact XML export flag
    void xmlMode(bool compact)
    {
        m_compactXmlMode = compact;
    }

    /// @brief Adds a new field int the list
    ///
    /// Creates and returns a new field.
    /// @param fname const char *, field name
    /// @param checkDuplicates bool, if true check if the field already exists in the list
    /// @returns new field reference
    Field& push_back(const char *fname,bool checkDuplicates);

    /// @brief Adds a new field int the list without creating a new copy of the field.
    ///
    /// This method is useful if you create a new field with the new() operator.
    /// You shouldn't delete such fields manually - they would be maintained by CFieldList class.
    /// @param fld CField *, field name
    /// @returns new field reference
    Field& push_back(Field *fld);

    /// @brief Finds a field by the field name
    ///
    /// Fast field lookup using std::map.
    /// @param fname const char *, field name
    /// @returns CField pointer, or 0L if not found
    Field* fieldByName(const char * fname) const;

    /// @brief Field access by field index, non-const version
    ///
    /// @param index uint32_t, field index
    /// @returns field reference
    Field& operator [](uint32_t index)
    {
        return *(Field *) m_list[index];
    }

    /// @brief Field access by field index, non-const version
    ///
    /// @param index int32_t, field index
    /// @returns field reference
    Field& operator [](int32_t index)
    {
        return *(Field *) m_list[size_t(index)];
    }

    /// @brief Field access by field index, const version
    /// @param index uint32_t, field index
    /// @returns field reference
    const Field& operator [](uint32_t index) const
    {
        return *(Field *) m_list[size_t(index)];
    }

    /// @brief Field access by field index, const version
    /// @param index int32_t, field index
    /// @returns field reference
    const Field& operator [](int32_t index) const
    {
        return *(Field *) m_list[size_t(index)];
    }

    /// @brief Field access by field name, non-const version
    /// @param fname const char *, field name
    /// @returns field reference
    Field& operator [](const char *fname)
    {
        return *fieldByName(fname);
    }

    /// Field access by field name, const version
    /// @param fname const char *, field name
    /// @returns field reference
    const Field& operator [](const char *fname) const
    {
        return *fieldByName(fname);
    }

    /// @brief Field access by field name, non-const version
    /// @param fname const std::string&, field name
    /// @returns field reference
    Field& operator [](const std::string& fname)
    {
        return *fieldByName(fname.c_str());
    }

    /// @brief Field access by field name, const version
    /// @param fname const std::string&, field name
    /// @returns field reference
    const Field& operator [](const std::string& fname) const
    {
        return *fieldByName(fname.c_str());
    }

    /// @brief Sets user data
    ///
    /// User data is usually a pointer to some outside memory object,
    /// or an index (id) of some object. CFieldList doesn't maintain this pointer, just keeps it
    /// as a tag.
    /// @param data void *, a user-defined data
    void user_data(void *data)
    {
        m_userData = data;
    }

    /// @brief Returns user data
    void* user_data() const
    {
        return m_userData;
    }

    /// @brief Sets the field stream iterator to the first field
    ///
    /// This method is useful if you're using stream access to fields
    void rewind()
    {
        m_fieldStreamItor = m_list.begin();
    }

    /// @brief Sets the field stream iterator to the next field.
    ///
    /// After the last field is reached, the iterator is switched to the first field
    /// @returns current field
    Field& next();

    /// @brief Exports data into XML node
    ///
    /// @see setXmlMode() for details.
    /// @param xml const XMLNode&, an XML node to store fields into
    void toXML(XMLNode& xml) const;
};
}

/// @brief Streamed field output
///
/// The data is read from the current field,
/// and then next field becomes current. The rewind() method is called
/// automatically to reset the field iterator to the first field upon
/// query open() or fetch() method calls.
/// @param fieldList CFieldList&, a list of fields to assign
/// @param data const bool&, a variable to read current field to
SP_EXPORT sptk::FieldList& operator >> (sptk::FieldList& fieldList, bool& data);

/// @brief Streamed field output
///
/// The data is read from the current field,
/// and then next field becomes current. The rewind() method is called
/// automatically to reset the field iterator to the first field upon
/// query open() or fetch() method calls.
/// @param fieldList CFieldList&, a list of fields to assign
/// @param data const std::string&, a variable to read current field to
SP_EXPORT sptk::FieldList& operator >> (sptk::FieldList& fieldList, std::string& data);

/// @brief Streamed field output
///
/// The data is read from the current field,
/// and then next field becomes current. The rewind() method is called
/// automatically to reset the field iterator to the first field upon
/// query open() or fetch() method calls.
/// @param fieldList CFieldList&, a list of fields to assign
/// @param data int&, a variable to read current field to
SP_EXPORT sptk::FieldList& operator >> (sptk::FieldList& fieldList, int& data);

/// @brief Streamed field output
///
/// The data is read from the current field,
/// and then next field becomes current. The rewind() method is called
/// automatically to reset the field iterator to the first field upon
/// query open() or fetch() method calls.
/// @param fieldList CFieldList&, a list of fields to assign
/// @param data double&, a variable to read current field to
SP_EXPORT sptk::FieldList& operator >> (sptk::FieldList& fieldList, double& data);

/// @brief Streamed field output
///
/// The data is read from the current field,
/// and then next field becomes current. The rewind() method is called
/// automatically to reset the field iterator to the first field upon
/// query open() or fetch() method calls.
/// @param fieldList CFieldList&, a list of fields to assign
/// @param data CDateTime, a data to assign to current parameter
SP_EXPORT sptk::FieldList& operator >> (sptk::FieldList& fieldList, sptk::DateTime& data);

/// @brief Streamed field output
///
/// The data is read from the current field,
/// and then next field becomes current. The rewind() method is called
/// automatically to reset the field iterator to the first field upon
/// query open() or fetch() method calls.
/// @param fieldList CFieldList&, a list of fields to assign
/// @param data const CBuffer&, a variable to read current field to
SP_EXPORT sptk::FieldList& operator >> (sptk::FieldList& fieldList, sptk::Buffer& data);

/// @brief Streamed fields output
///
/// Exports the data from fields into XML node
/// @param fieldList CFieldList&, a list of fields to assign
/// @param fields const XMLNode&, an XML node variable to read fields into
SP_EXPORT sptk::FieldList& operator >> (sptk::FieldList& fieldList, sptk::XMLNode& fields);

/// @}

#endif
