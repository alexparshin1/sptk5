/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CDataSource.h - description                            ║
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

#ifndef __CDATASOURCE_H__
#define __CDATASOURCE_H__

#include <sptk5/CVariant.h>
#include <sptk5/CFieldList.h>
#include <sptk5/cxml>

class Fl_Group;

namespace sptk {

/// @addtogroup utility Utility Classes
/// @{

/// @brief Universal data source for many SPTK widgets.
///
/// It's designed as a base class for multiple datasources available in SPTK.
/// The main idea is to provide the simple interface that allows to open the datasource
/// with certain parameters and read or write the datasource fields. And don't you forget to close it :)
class SP_EXPORT CDataSource
{
    friend class Fl_Group;

protected:
    /// @brief Loads datasource data. Should be implemented in derived class
    virtual bool loadData()
    {
        return true;
    }

    /// @brief Saves data from datasource. Should be implemented in derived class
    virtual bool saveData()
    {
        return true;
    }

public:
    /// @brief Default constructor
    CDataSource()
    {}

    /// @brief Destructor
    virtual ~CDataSource()
    {}

public:
    /// @brief Field access by the field index, const version.
    ///
    /// Purely virtual. Should be implemented in derived class
    /// @param fieldIndex uint32_t, field index
    /// @returns field reference
    virtual const CField& operator [] (uint32_t fieldIndex) const = 0;

    /// @brief Field access by the field index, non-const version
    ///
    /// Purely virtual. Should be implemented in derived class
    /// @param fieldIndex uint32_t, field index
    /// @returns field reference
    virtual CField&       operator [] (uint32_t fieldIndex) = 0;

    /// @brief Field access by the field name, const version.
    ///
    /// Purely virtual. Should be implemented in derived class
    /// @param fieldName const char *, field name
    /// @returns field reference
    virtual const CField& operator [] (const char *fieldName) const = 0;

    /// @brief Field access by the field name, const version.
    ///
    /// Purely virtual. Should be implemented in derived class
    /// @param fieldName const char *, field name
    /// @returns field reference
    virtual CField&       operator [] (const char *fieldName) = 0;

    /// @brief Returns field count in the datasource.
    ///
    /// Purely virtual. Should be implemented in derived class
    /// @returns field count
    virtual uint32_t fieldCount() const = 0;
public:

    /// @brief Returns record count in the datasource.
    ///
    /// Purely virtual. Should be implemented in derived class
    /// @returns record count
    virtual uint32_t recordCount() const = 0;

    /// @brief Returns user_data associated with the datasource.
    virtual void* user_data() const 
    {
        return 0L;
    }
public:
    /// @brief Reads the field by name from the datasource.
    ///
    /// Purely virtual. Should be implemented in derived class
    /// @param fieldName const char *, field name
    /// @param fieldValue CVariant, field value
    virtual bool readField(const char *fieldName, Variant& fieldValue) = 0;

    /// @brief Writes the field by name from the datasource.
    ///
    /// Purely virtual. Should be implemented in derived class
    /// @param fieldName const char *, field name
    /// @param fieldValue CVariant, field value
    virtual bool writeField(const char *fieldName, const Variant& fieldValue) = 0;
public:

    /// @brief Opens the datasource. Implemented in derved class.
    virtual bool open() THROWS_EXCEPTIONS
    {
        return false;
    }

    /// @brief Closes the datasource. Implemented in derved class.
    virtual bool close()
    {
        return false;
    }

    /// @brief Moves to the first record of the datasource. Implemented in derved class.
    virtual bool first()
    {
        return false;
    }

    /// @brief Moves to the next record of the datasource. Implemented in derved class.
    virtual bool next()
    {
        return false;
    }

    /// @brief Moves to the prior record of the datasource. Implemented in derved class.
    virtual bool prior()
    {
        return false;
    }

    /// @brief Moves to the last record of the datasource. Implemented in derved class.
    virtual bool last()
    {
        return false;
    }

    /// @brief Moves to the specified record position of the datasource. Implemented in derved class.
    virtual bool find(Variant /*position*/)
    {
        return false;
    }

    /// @brief Returns true if there are no more records in the datasource. Implemented in derved class.
    virtual bool eof() const
    {
        return false;
    }

    /// @brief Loads data into the datasource
    bool load();

    /// @brief Saves data from the datasource
    bool save();

    /// @brief Saves dataset row data into XML
    ///
    /// If the compactXmlMode is true, the node would have fields presented as attributues.
    /// Otherwise, the fields are stored as subnodes.
    /// @param node CXmlNode&, the XML node to fill in
    /// @param compactXmlMode bool, compact XML flag
    void rowToXML(CXmlNode& node, bool compactXmlMode) const;

    /// @brief Saves data into XML
    ///
    /// Opens the dataset, reads every row, and closes dataset.
    /// For every row in the dataset, creates the node with the name nodeName.
    /// If the compactXmlMode is true, the nodes would have fields presented as attributues.
    /// Otherwise, the fields are stored as subnodes.
    /// @param parentNode CXmlNode&, the XML node to add subnodes to
    /// @param nodeName std::string, the name for subnodes
    /// @param compactXmlMode bool, compact XML flag
    virtual void toXML(CXmlNode& parentNode, std::string nodeName, bool compactXmlMode);
};

/// @}
}
#endif
