/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2020 by Alexey Parshin. All rights reserved.    ║
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

#ifndef __SPTK_ODBCENV_H__
#define __SPTK_ODBCENV_H__

#include <sptk5/sptk.h>

#if HAVE_ODBC == 1

#include <sptk5/Strings.h>

#ifdef WIN32
    #include <winsock2.h>
    #include <windows.h>
    #include <sys/types.h>
    #include <time.h>
#else
    #define LPCVOID  const void *
#endif

#include <sqlext.h>
#include <cassert>

#include <sptk5/db/QueryParameterList.h>
#include <mutex>

namespace sptk
{

/**
 * @addtogroup Database Database Support
 * @{
 */

class ODBCEnvironment;
class ODBCConnectionBase;

/**
 * @brief ODBC base
 *
 * Base class for all ODBC classes
 */
class SP_DRIVER_EXPORT ODBCBase : public std::mutex
{
	friend class ODBCConnection;

public:

    /**
     * Destructor
     */
    ~ODBCBase() = default;

    /**
     * Throws the exception
     */
    [[noreturn]] static void exception(const String& text, int line);

protected:

    /**
     * Constructor
     */
    ODBCBase() = default;

private:

    /**
     * Assignment operator, disabled
     */
    ODBCBase& operator=(const ODBCBase& d);

    /**
     * Copy constructor, disabled
     */
    ODBCBase(const ODBCBase&);
};

/**
 * @brief ODBC environment
 *
 * Environment is only used by ODBCConnection class
 */
class SP_DRIVER_EXPORT ODBCEnvironment : public ODBCBase
{
    friend class ODBCConnectionBase;

public:

    /**
     * Returns enviromment handle
     */
    SQLHENV handle() const
    {
        return m_hEnvironment;
    }

    /**
     * Deleted copy constructor
     */
    ODBCEnvironment(const ODBCEnvironment&) = delete;

    /**
     * Destructor
     */
    ~ODBCEnvironment();

    /**
     * Deleted copy assignment
     */
    ODBCEnvironment& operator = (const ODBCEnvironment&) = delete;

protected:

    /**
     * Constructor
     */
    ODBCEnvironment() = default;

    /**
     * Allocates enviromment handle
     */
    void allocEnv();

    /**
     * Deallocates enviromment handle
     */
    void freeEnv();

    /**
     * Is enviromment handle allocated?
     */
    bool valid() const
    {
        return m_hEnvironment != SQL_NULL_HENV;
    }

private:
    /**
     * ODBC environment handle
     */
    SQLHENV m_hEnvironment {SQL_NULL_HENV};
};

/**
 * @brief ODBC connection
 *
 * Class ODBCConnection represents the ODBC connection to a database.
 */
class SP_DRIVER_EXPORT ODBCConnectionBase : public ODBCBase
{
public:

    /**
     * Default constructor
     */
    ODBCConnectionBase() = default;

    /**
     * Default destructor
     */
    ~ODBCConnectionBase();

    /**
     * Allocates connection
     */
    void allocConnect();

    /**
     * Deallocates connection
     */
    void freeConnect();

    /**
     * Connects to the database passing ODBC connection string.
     * The full connection string is returned in FinalConnectionString.
     */
    void connect(const String& ConnectionString, String& FinalConnectionString, bool EnableDriverPrompt = false);

    /**
     * Disconnects from the database passing ODBC connection string.
     */
    void disconnect();

    /**
     * Returns the connection handle
     */
    SQLHDBC handle() const
    {
        return m_hConnection;
    }

    /**
     * Returns true if the connection is active
     */
    bool isConnected()
    {
        std::lock_guard<std::mutex> lock(*this);
        return m_connected;
    }

    /**
     * Sets the connection option
     */
    void setConnectOption(UWORD fOption, UDWORD vParam);

    /**
     * Returns the ODBC connection string for the active connection
     */
    String connectString()
    {
        std::lock_guard<std::mutex> lock(*this);
        return m_connectString;
    }

    /**
     * Returns the ODBC driver description string for the active connection
     */
    String driverDescription()
    {
        std::lock_guard<std::mutex> lock(*this);
        return m_driverDescription;
    }

    /**
     * Begins transaction
     */
    void beginTransaction();

    /**
     * Controls transaction
     */
    void transact(uint16_t fType);

    /**
     * Commits transaction
     */
    void commit()
    {
        transact(SQL_COMMIT);
    }

    /**
     * Rollbacks transaction
     */
    void rollback()
    {
        transact(SQL_ROLLBACK);
    }

    /**
     * Returns the only environment needed
     */
    static ODBCEnvironment& getEnvironment();

    /**
     * Retrieves an error information for user action name
     * @returns ODBC driver error message with the user action
     */
    String errorInformation(const char* action);

protected:
    /**
     * Is connection active?
     */
    bool valid() const
    {
        return m_hConnection != SQL_NULL_HDBC;
    }

    /**
     * Execute query in current connection
     * @param query             Query to execute
     */
    void execQuery(const char* query);

private:

    ODBCEnvironment&    m_cEnvironment {getEnvironment()};  ///< ODBC environment
    SQLHDBC             m_hConnection {SQL_NULL_HDBC};      ///< ODBC connection handle
    bool                m_connected {false};                ///< Is connection active?
    String              m_connectString;                    ///< ODBC connection string
    String              m_driverDescription;                ///< Driver description, filled in during the connection to the DSN
};

/**
 * Removes excessive driver information from error message
 */
String removeDriverIdentification(const char* error);

/**
 * @}
 */
}
#endif

#endif
