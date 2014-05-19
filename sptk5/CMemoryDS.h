/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CMemoryDS.h  -  description
                             -------------------
    begin                : Mon Apr 17 2000
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
   This library is free software; you can redistribute it and/or modify it
   under the terms of the GNU Library General Public License as published by
   the Free Software Foundation; either version 2 of the License, or (at
   your option) any later version.

   This library is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU Library
   General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

   Please report all bugs and problems to "alexeyp@gmail.com"
 ***************************************************************************/

#ifndef __CMEMORYDS_H__
#define __CMEMORYDS_H__

#include <sptk5/sptk.h>
#include <sptk5/CFieldList.h>
#include <sptk5/CDataSource.h>
#include <sptk5/CException.h>
#include <vector>

namespace sptk {

/// @addtogroup gui GUI Classes
/// @{

/// @brief Base (memory) datasource
///
/// Class CMemoryDS implements a special case of the datasource when the data
/// can be loaded all at once, in the datasource open() operation. It's a base class
/// for several actual datasources.
class CMemoryDS : public CDataSource  {
protected:
    std::vector<CFieldList *>  m_list;            ///< Internal list of the dataset records
    CFieldList*                m_current;         ///< Current record in the dataset.
    uint32_t                   m_currentIndex;    ///< The index of the current record.
    bool                       m_eof;             ///< EOF flag for sequentual reading first(),next()..next().

protected:

    /// Default constructor is protected, to prevent creating of the instance of that class
    CMemoryDS() : CDataSource(), m_current(0L), m_currentIndex(0) { }

public:

    /// Destructor
    virtual ~CMemoryDS() {
        close();
    }

    /// Clears all the records
    virtual void clear();

    /// Field access by the field index, const version.
    /// @param fieldIndex int, field index
    /// @returns field reference
    virtual const CField& operator [] (uint32_t fieldIndex) const;

    /// Field access by the field index, non-const version.
    /// @param fieldIndex int, field index
    /// @returns field reference
    virtual CField&       operator [] (uint32_t fieldIndex);

    /// Field access by the field name, const version.
    /// @param fieldName const char *, field name
    /// @returns field reference
    virtual const CField& operator [] (const char *fieldName) const;

    /// Field access by the field name, non-const version.
    /// @param fieldName const char *, field name
    /// @returns field reference
    virtual CField&       operator [] (const char *fieldName);

    /// Returns user_data associated with the datasource.
    virtual void             *user_data() const {
        return m_current->user_data();
    }

    /// Returns field count in the datasource.
    /// @returns field count
    virtual uint32_t           fieldCount() const;

    /// Returns record count in the datasource.
    /// @returns record count
    virtual uint32_t           recordCount() const;

    /// Reads the field by name from the datasource.
    /// @param fieldName const char *, field name
    /// @param fieldValue CVariant, field value
    virtual bool              readField(const char *fieldName,CVariant& fieldValue);

    /// Writes the field by name from the datasource.
    /// @param fieldName const char *, field name
    /// @param fieldValue CVariant, field value
    virtual bool              writeField(const char *fieldName,const CVariant& fieldValue);

    /// Opens the datasource. Implemented in derved class.
    virtual bool              open() THROWS_EXCEPTIONS {
        throw CException("Not implemented yet");
    }

    /// Closes the datasource.
    virtual bool              close();

    /// Moves to the first record of the datasource.
    virtual bool              first();

    /// Moves to the next record of the datasource.
    virtual bool              next();

    /// Moves to the prior record of the datasource.
    virtual bool              prior();

    /// Moves to the last record of the datasource.
    virtual bool              last();

    /// Finds the record by the record position (defined by record's user_data or key).
    virtual bool              find(CVariant position);

    /// Returns true if there are no more records in the datasource. Implemented in derved class.
    virtual bool              eof() const {
        return m_eof;
    }
};
/// @}
}
#endif
