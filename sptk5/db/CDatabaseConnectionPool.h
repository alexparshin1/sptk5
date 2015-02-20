/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDatabaseConnectionPool.h  -  description
                             -------------------
    begin                : Sun Mar 11 2012
    copyright            : (C) 1999-2014 by Alexey Parshin. All rights reserved.
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

#ifndef __CDATABASECONNECTIONLOADER_H__
#define __CDATABASECONNECTIONLOADER_H__

#include <sptk5/db/CDatabaseConnection.h>
#include <sptk5/db/CDatabaseConnectionString.h>
#include <sptk5/CCaseInsensitiveCompare.h>
#include <sptk5/threads/CSynchronizedList.h>
#include <sptk5/threads/CSynchronizedQueue.h>

namespace sptk
{

/// @addtogroup Database Database Support
/// @{

typedef CDatabaseConnection* CCreateDriverInstance(const char*);
typedef void CDestroyDriverInstance(CDatabaseConnection*);

#ifdef WIN32
    typedef HMODULE CDriverHandle;                   ///< Windows: Driver DLL handle type
#else
    typedef void*   CDriverHandle;                   ///< Unix: Driver shared library handle type
#endif

/// @brief Information about loaded database driver
struct SP_EXPORT CDatabaseDriver
{
    CDriverHandle                               m_handle;               ///< Driver SO/DLL handle after load
    CCreateDriverInstance*                      m_createConnection;     ///< Function that creates driver instances
    CDestroyDriverInstance*                     m_destroyConnection;    ///< Function that destroys driver instances
};

/// @brief Database driver loader
///
/// Loads and initializes SPTK database driver by request.
/// Already loaded drivers are cached.
class SP_EXPORT CDatabaseConnectionPool : public CSynchronized, public CDatabaseConnectionString
{
    CDatabaseDriver*                            m_driver;               ///< Database driver
protected:
    CCreateDriverInstance*                      m_createConnection;     ///< Function that creates driver instances
    CDestroyDriverInstance*                     m_destroyConnection;    ///< Function that destroys driver instances
    unsigned                                    m_maxConnections;       ///< Maximum number of connections in the pool
    CSynchronizedList<CDatabaseConnection*>     m_connections;          ///< List all connections
    CSynchronizedQueue<CDatabaseConnection*>    m_pool;                 ///< Connection pool

    /// @brief Loads database driver
    ///
    /// First successfull driver load places driver into driver cache.
    void load() THROWS_EXCEPTIONS;

public:
    /// @brief Constructor
    ///
    /// Database connection string is the same for all connections,
    /// created with this object.
    /// @param connectionString std::string, Database connection string
    /// @param maxConnections unsigned, Maximum number of connections in the pool
    CDatabaseConnectionPool(std::string connectionString, unsigned maxConnections=100);

    /// @brief Destructor
    ///
    /// Closes and destroys all created connections
    ~CDatabaseConnectionPool();

    /// @brief Creates database connection
    CDatabaseConnection* createConnection() THROWS_EXCEPTIONS;

    /// @brief Returns used database connection back to the pool
    /// @param connection CDatabaseConnection*, Database that is no longer in use and may be returned to the pool
    void releaseConnection(CDatabaseConnection* connection);

    /// @brief Destroys connection
    /// @param connection CDatabaseConnection*, destroys the driver instance
    /// @param unlink bool, should always be true for any eternal use
    void destroyConnection(CDatabaseConnection* connection,bool unlink=true);
};
/// @}
}
#endif
