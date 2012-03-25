/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDatabaseConnectionString.h  -  description
                             -------------------
    begin                : Sun Mar 25 2012
    copyright            : (C) 2000-2012 by Alexey Parshin. All rights reserved.
    email                : alexeyp@gmail.com
 ***************************************************************************/

/***************************************************************************
  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice,
    this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.
  * Neither the name of the <ORGANIZATION> nor the names of its contributors
    may be used to endorse or promote products derived from this software
    without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
  OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
  EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
  PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
  PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
  LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 ***************************************************************************/

#ifndef __CDATABASECONNECTIONSTRING_H__
#define __CDATABASECONNECTIONSTRING_H__

#include <sptk5/sptk.h>
#include <sptk5/CException.h>

namespace sptk
{

/// @addtogroup Database Database Support
/// @{


/// @brief Database Connection String
///
/// Database connection string includes driver name ('odbc', 'sqlite3', etc) as a protocol name,
/// and username, password, server name in a traditional form. Database name is optionally defined
/// after server name and '/' delimiter.
///
/// Example:
///   drivername://[username:password]@servername[:port]/databasename
///
/// Some driver-specific parameters may be passed after '?':
///   drivername://username:password@servername/databasename?timeout=10&reconnect=30
///
/// This class is thread-safe.
class SP_EXPORT CDatabaseConnectionString
{
public:
    typedef std::map<std::string,std::string> Parameters;

protected:
    /// @brief Parses connection string
    void parse() throw (CDatabaseException);

    /// @brief Parses connection string
    /// @param ptr char*, Parameters string
    void parseParameters(char* ptr) throw (CDatabaseException);

    std::string     m_connectionString;         ///< Database connection string
    std::string     m_driverName;               ///< Database driver name
    std::string     m_hostName;                 ///< Database server host name
    uint16_t        m_portNumber;               ///< Database server port number
    std::string     m_userName;                 ///< Database user name
    std::string     m_password;                 ///< Database user name
    std::string     m_databaseName;             ///< Database user password
    Parameters      m_parameters;               ///< Optional parameters

public:
    /// @brief Constructor
    /// @param connectionString std::string, Database connection string
    CDatabaseConnectionString(std::string connectionString) :
        m_connectionString(connectionString),
        m_portNumber(0)
    {
        parse();
    }

    /// @brief Returns connection string
    const std::string& str() const
    {
        return m_connectionString;
    }

    /// @brief Returns driver name
    const std::string& driverName() const
    {
        return m_driverName;
    }

    /// @brief Returns host name
    const std::string& hostName() const
    {
        return m_hostName;
    }

    /// @brief Returns user name
    const std::string& userName() const
    {
        return m_userName;
    }

    /// @brief Returns user password
    const std::string& password() const
    {
        return m_password;
    }

    /// @brief Returns database name
    const std::string& databaseName() const
    {
        return m_databaseName;
    }

    /// @brief Returns server port number
    uint16_t portNumber() const
    {
        return m_portNumber;
    }

    /// @brief Returns optional database parameters
    const Parameters& parameters() const
    {
        return m_parameters;
    }
};
/// @}
}
#endif
