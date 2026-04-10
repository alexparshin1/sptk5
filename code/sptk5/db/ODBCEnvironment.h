/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/sptk.h>

#ifdef HAVE_ODBC

#ifdef WIN32
#include <sys/types.h>
#include <time.h>
#include <winsock2.h>

#include <windows.h>
#else
#define LPCVOID const void*
#endif

#include <sqlext.h>

#include <mutex>

namespace sptk {
/**
 * @addtogroup Database Database Support.
 * @{
 */

class ODBCEnvironment;

class ODBCConnectionBase;

/**
 * @brief ODBC base.
 *
 * Base class for all ODBC classes.
 */
class SP_DRIVER_EXPORT ODBCBase
    : public std::mutex
{
    friend class ODBCConnection;

public:
    static constexpr const char* cantSetConnectOption = "Can't set connect option";
    static constexpr const char* cantEndTranscation = "Can't end transaction";
    static constexpr const char* cantGetInformation = "Can't get connect information";

    /**
     * @brief Destructor.
     */
    ~ODBCBase() = default;

    /**
     * @brief Assignment operator, disabled.
     */
    ODBCBase& operator=(const ODBCBase& d) = delete;

    /**
     * @brief Copy constructor, disabled.
     */
    ODBCBase(const ODBCBase&) = delete;

protected:
    /**
     * @brief Constructor.
     */
    ODBCBase() = default;
};

/**
 * @brief ODBC environment.
 *
 * Environment is only used by ODBCConnection class.
 */
class SP_DRIVER_EXPORT ODBCEnvironment
    : public ODBCBase
{
    friend class ODBCConnectionBase;

public:
    /**
     * @brief Returns enviromment handle.
     */
    [[nodiscard]] SQLHENV handle() const
    {
        return m_hEnvironment.get();
    }

protected:
    /**
     * @brief Constructor.
     */
    ODBCEnvironment() = default;

    /**
     * @brief Allocates enviromment handle.
     */
    void allocEnv();

    /**
     * @brief Is the environment handle allocated?
     */
    [[nodiscard]] bool valid() const
    {
        return m_hEnvironment != nullptr;
    }

private:
    /**
     * @brief ODBC environment handle.
     */
    std::shared_ptr<uint8_t> m_hEnvironment;
};

/**
 * @brief ODBC connection.
 *
 * Class ODBCConnection represents the ODBC connection to a database.
 */
class SP_DRIVER_EXPORT ODBCConnectionBase
    : public ODBCBase
{
public:
    /**
     * @brief Allocates connection.
     */
    void allocConnect();

    /**
     * @brief Deallocates connection.
     */
    void freeConnect();

    /**
     * @brief Connects to the database passing ODBC connection string.
     * The full connection string is returned in FinalConnectionString.
     */
    void connect(const String& ConnectionString, String& FinalConnectionString, bool EnableDriverPrompt = false);

    /**
     * @brief Disconnects from the database passing ODBC connection string.
     */
    void disconnect();

    /**
     * @brief Returns the connection handle.
     */
    [[nodiscard]] SQLHDBC handle() const
    {
        return (SQLHDBC) m_hConnection.get();
    }

    /**
     * @brief Returns true if the connection is active.
     */
    bool isConnected()
    {
        std::scoped_lock lock(*this);
        return m_connected;
    }

    /**
     * @brief Sets the connection option.
     */
    void setConnectOption(UWORD fOption, UDWORD vParam);

    /**
     * @brief Returns the ODBC connection string for the active connection.
     */
    String connectString()
    {
        std::scoped_lock lock(*this);
        return m_connectString;
    }

    /**
     * @brief Returns the ODBC driver description string for the active connection.
     */
    String driverDescription()
    {
        std::scoped_lock lock(*this);
        return m_driverDescription;
    }

    /**
     * @brief Begins transaction.
     */
    void beginTransaction();

    /**
     * @brief Controls transaction.
     */
    void transact(uint16_t fType);

    /**
     * @brief Commits transaction.
     */
    void commit()
    {
        transact(SQL_COMMIT);
    }

    /**
     * @brief Rollbacks transaction.
     */
    void rollback()
    {
        transact(SQL_ROLLBACK);
    }

    /**
     * @brief Returns the only environment needed.
     */
    static ODBCEnvironment& getEnvironment()
    {
        return m_env;
    }

    /**
     * @brief Retrieves error information for the user action name.
     * @returns ODBC driver error message with the user action.
     */
    String errorInformation(const char* action) const;

protected:
    /**
     * @brief Is the connection active?
     */
    [[nodiscard]] bool valid() const
    {
        return m_hConnection != SQL_NULL_HDBC;
    }

    /**
     * @brief Execute query in current connection.
     * @param query             Query to execute.
     */
    void execQuery(const char* query);

private:
    ODBCEnvironment&         m_cEnvironment {getEnvironment()}; ///< ODBC environment.
    std::shared_ptr<uint8_t> m_hConnection;                     ///< ODBC connection handle.
    bool                     m_connected {false};               ///< Is connection active?.
    String                   m_connectString;                   ///< ODBC connection string.
    String                   m_driverDescription;               ///< Driver description, filled in during the connection to the DSN.
    static ODBCEnvironment   m_env;
};

/**
 * @brief Removes excessive driver information from the error message.
 */
String removeDriverIdentification(const char* error);

/**
 * @}
 */
} // namespace sptk
#endif
