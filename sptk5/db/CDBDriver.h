/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDBDriver.h  -  description
                             -------------------
    begin                : Sun Mar 11 2012
    copyright            : (C) 2000-2012 by Alexey Parshin. All rights reserved.
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

#ifndef __CDBDRIVER_H__
#define __CDBDRIVER_H__

#include <sptk5/sptk.h>
#include <sptk5/CStrings.h>
#include <sptk5/threads/CSynchronizedCode.h>
#include <sptk5/CVariant.h>
#include <sptk5/CFileLog.h>

#include <vector>

namespace sptk
{

/// @addtogroup Database Database Support
/// @{

/// @brief Database driver
///
/// Implements a thread-safe connection to general database. It is used
/// as a base class for the particular database driver, CODBCDatabase,
/// for instance.
class SP_EXPORT CDBDriver: public CSynchronized
{
    bool        m_connected;
    std::string m_connString;
public:
    CDBDriver(std::string connectionString);
    virtual ~CDBDriver();
    void open(std::string connectionString = "") throw (CException);
    void close() throw (CException);
    virtual std::string connectionString() const
    {
        return m_connString;
    }
    void objectList(CStrings& objects) throw (std::exception);
};
/// @}
}
#endif
