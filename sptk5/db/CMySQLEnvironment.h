/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CMySQLConnection.h  -  description
                             -------------------
    begin                : Sat November 17 2012
    copyright            : (C) 1999-2013 by Alexey Parshin. All rights reserved.
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

#ifndef __CORACLEENVIRONMENT_H__
#define __CORACLEENVIRONMENT_H__

#include <sptk5/db/CDatabaseConnection.h>

#if HAVE_ORACLE == 1

#include <occi.h>

namespace sptk
{

/// @addtogroup Database Database Support
/// @{

/// @brief MySQL Environment
///
/// Allows creating and terminating connections
class CMySQLEnvironment
{
    MYSQL* m_handle;
public:
    /// @brief Constructor
    CMySQLEnvironment();

    /// @brief Destructor
    ~CMySQLEnvironment();

    /// @brief Returns environment handle
    MYSQL* handle() const
    {
        return m_handle;
    }

    /// @brief Returns client version
    std::string clientVersion() const;

    /// @brief Creates new database connection
    /// @param connectionString CDatabaseConnectionString&, Connection parameters
    MYSQL* createConnection(CDatabaseConnectionString& connectionString);

    /// @brief Terminates database connection
    /// @param connection oracle::occi::Connection*, MySQL connection
    void terminateConnection(oracle::occi::Connection*);
};

/// @}
}

#endif

#endif
