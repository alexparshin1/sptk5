/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CDatabaseDriver.h  -  description
                             -------------------
    begin                : Wed Dec 15 1999
    copyright            : (C) 1999-2012 by Alexey Parshin. All rights reserved.
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

#ifndef __CDATABASEDRIVER_H__
#define __CDATABASEDRIVER_H__

#include <sptk5/sptk.h>
#include <sptk5/CStrings.h>
#include <sptk5/threads/CSynchronizedCode.h>
#include <sptk5/CVariant.h>
#include <sptk5/CFileLog.h>

#include <vector>

namespace sptk {

/// @addtogroup Database Database Support
/// @{

class CQuery;

/// @brief Types of the objects for CDatabaseDriver::listObjects method
enum CDbObjectType
{
    DOT_TABLES,         ///< Tables
    DOT_VIEWS,          ///< Views
    DOT_PROCEDURES      ///< Stored procedures
};

/// @brief Structure to store statistical data about query calls
struct CCallStatistic
{
    double      m_duration;   ///< Total calls duration, sec
    unsigned    m_calls;      ///< Total number of calls
    std::string m_sql;        ///< The last used query sql

    /// @brief Default constructor
    CCallStatistic()
    {
        m_duration = 0;
        m_calls = 0;
    }

    /// @brief Constructor
    /// @param duration double, Total calls duration, sec
    /// @param calls unsigned, Total number of calls
    /// @param sql const std::string&, The last used query sql
    CCallStatistic(double duration, unsigned calls, const std::string& sql)
    {
        m_duration = duration;
        m_calls = calls;
        m_sql = sql;
    }

    /// @brief Copy constructor
    CCallStatistic(const CCallStatistic& cs)
    {
        m_duration = cs.m_duration;
        m_calls = cs.m_calls;
        m_sql = cs.m_sql;
    }
};

/// @brief Map to store statistical data about query calls
///
/// The map index is the query creation location in (file:line) format
typedef std::map<std::string, CCallStatistic> CCallStatisticMap;

/// @brief Database connector
///
/// Implements a thread-safe connection to general database. It is used
/// as a base class for actual database driver classes.
class SP_EXPORT CDatabaseDriver: public CSynchronized
{
    typedef std::vector<CQuery*> CQueryVector;
    friend class CQuery;

protected:

    CQueryVector m_queryList;      ///< The list of queries that use this database
    std::string  m_connString;     ///< The ODBC driver connection string
    bool         m_inTransaction;  ///< The in-transaction flag
    CBaseLog*    m_log;            ///< Log for the database events (optional)
    std::string  m_objectName;     ///< Object name for logs and error messages

    /// @brief Attaches (links) query to the database
    bool linkQuery(CQuery *q);

    /// @brief Unlinks query from the database
    bool unlinkQuery(CQuery *q);

    CCallStatisticMap m_queryStatisticMap; ///< Map of query creation location to statistical information

    /// @brief Clears statistical information about query calls
    void clearStatistics();

    /// @brief Adds statistical information about query calls
    /// @param location const std::string&, query creation location (file:line)
    /// @param totalDuration double, total execution time for query calls, sec
    /// @param totalCalls unsigned, total number of query calls
    /// @param sql const std::string&, last sql used by query
    void addStatistics(const std::string& location, double totalDuration, unsigned totalCalls, const std::string& sql);

protected:
    // These methods get access to CQuery's protected members
    void querySetAutoPrep(CQuery *q, bool pf);       ///< Sets internal CQuery m_autoPrepare flag
    void querySetStmt(CQuery *q, void *stmt);        ///< Sets internal CQuery statement handle
    void querySetConn(CQuery *q, void *conn);        ///< Sets internal CQuery connection handle
    void querySetPrepared(CQuery *q, bool pf);       ///< Sets internal CQuery m_prepared flag
    void querySetActive(CQuery *q, bool af);         ///< Sets internal CQuery m_active flag
    void querySetEof(CQuery *q, bool eof);           ///< Sets internal CQuery m_eof flag

    // These methods implement the actions requested by CQuery
    virtual std::string queryError(const CQuery *query) const; ///< Retrieves an error (if any) after executing a statement
    virtual void queryAllocStmt(CQuery *query);      ///< Allocates an ODBC statement
    virtual void queryFreeStmt(CQuery *query);       ///< Deallocates an ODBC statement
    virtual void queryCloseStmt(CQuery *query);      ///< Closes an ODBC statement
    virtual void queryPrepare(CQuery *query);        ///< Prepares a query if supported by database
    virtual void queryUnprepare(CQuery *query);      ///< Unprepares a query if supported by database
    virtual void queryExecute(CQuery *query);        ///< Executes a statement
    virtual int queryColCount(CQuery *query);        ///< Counts columns of the dataset (if any) returned by query
    virtual void queryColAttributes(CQuery *query, int16_t column, int16_t descType, int32_t& value); ///< In a dataset returned by a query, retrieves the column attributes
    virtual void queryColAttributes(CQuery *query, int16_t column, int16_t descType, char *buff, int len); ///< In a dataset returned by a query, retrieves the column attributes
    virtual void queryBindParameters(CQuery *query); ///< Binds the parameters to the query
    virtual void queryOpen(CQuery *query);           ///< Opens the query for reading data from the query' recordset
    virtual void queryFetch(CQuery *query);          ///< Reads data from the query' recordset into fields, and advances to the next row. After reading the last row sets the EOF (end of file, or no more data) flag.

    /// @brief Returns parameter mark
    ///
    /// Parameter mark is generated from the parameterIndex.
    /// @param paramIndex unsigned, parameter index in SQL starting from 0
    virtual std::string paramMark(unsigned paramIndex)
    {
        return "?";
    }
protected:

    std::string m_driverDescription; ///< Driver description is filled by the particular driver.

    /// @brief Constructor
    ///
    /// Protected constructor prevents creating an instance of the
    /// CDatabaseDriver. Instead, it is possible to create an instance of derived
    /// classes.
    /// @param connectionString std::string, the connection string
    CDatabaseDriver(std::string connectionString);

    /// Stub function to throw an exception in case if the
    /// called method isn't implemented in the derived class
    void notImplemented(const char *methodName) const;

    void *queryHandle(CQuery *query) const;          ///< Retrieves internal query handle
    void queryHandle(CQuery *query, void *handle);    ///< Sets internal query handle

    /// @brief Opens the database connection.
    ///
    /// This method should be overwritten in derived classes
    /// @param connectionString std::string, the ODBC connection string
    virtual void openDatabase(std::string connectionString = "") throw (CDatabaseException);

    /// @brief Closes the database connection.
    ///
    /// This method should be overwritten in derived classes
    virtual void closeDatabase() throw (CDatabaseException);

    /// @brief Begins the transaction
    ///
    /// This method should be implemented in derived driver
    virtual void driverBeginTransaction() throw (CDatabaseException)
    {
        notImplemented("driverBeginTransaction");
    }

    /// @brief Ends the transaction
    ///
    /// This method should be implemented in derived driver
    /// @param commit bool, commit if true, rollback if false
    virtual void driverEndTransaction(bool commit) throw (CDatabaseException)
    {
        notImplemented("driverEndTransaction");
    }

    /// @brief Throws an exception
    ///
    /// Before exception is thrown, it is logged into the logfile (if the logfile is defined)
    /// @param method std::string, method name where error has occured
    /// @param error std::string, error text
    void logAndThrow(std::string method, std::string error) throw (CDatabaseException);

public:
    /// @brief Destructor
    ///
    /// Closes the database connection and all the connected queries.
    /// Releases all the database resources allocated during the connection.
    virtual ~CDatabaseDriver();

    /// @brief Opens the database connection
    ///
    /// If unsuccessful throws an exception.
    /// @param connectionString std::string, the ODBC connection string
    void open(std::string connectionString = "") throw (CDatabaseException);

    /// @brief Closes the database connection. If unsuccessful throws an exception.
    void close() throw (CDatabaseException);

    /// @brief Returns true if database is opened
    virtual bool active() const;

    /// @brief Returns the database connection handle
    virtual void* handle() const;

    /// @brief Returns the connection string
    virtual std::string connectionString() const
    {
        return m_connString;
    }

    /// @brief Returns the driver description
    virtual std::string driverDescription() const
    {
        return m_driverDescription;
    }

    /// @brief Begins the transaction
    void beginTransaction() throw (CDatabaseException);

    /// @brief Commits the transaction
    void commitTransaction() throw (CDatabaseException);

    /// @brief Rolls back the transaction
    void rollbackTransaction() throw (CDatabaseException);

    /// @brief Reports true if in transaction
    int inTransaction()
    {
        return m_inTransaction;
    }

    /// @brief Lists database objects
    ///
    /// Not implemented in CDatabaseDriver. The derived database class
    /// must provide its own implementation
    /// @param objectType CDbObjectType, object type to list
    /// @param objects CStrings&, object list (output)
    virtual void objectList(CDbObjectType objectType, CStrings& objects) throw (CDatabaseException) = 0;

    /// @brief Sets a log file for the database operations.
    ///
    /// If the database log is set, the database would log the events in CDatabaseDriver and CQuery objects
    /// into this log. To stop the logging, set the logFile parameter to NULL, or deactivate the log.
    /// @param logFile CBaseLog *, the log file object to use.
    void logFile(CBaseLog *logFile)
    {
        m_log = logFile;
    }

    /// @brief Returns a log file for the database operations.
    /// @returns current log file ptr, ot NULL if log file isn't set
    CBaseLog *logFile()
    {
        return m_log;
    }

    /// @brief Returns statistical information about query calls
    ///
    /// The information is collected between database open() and close() operations.
    /// Every open() operation resets the statistics
    const CCallStatisticMap& callStatistics() const
    {
        return m_queryStatisticMap;
    }

};
/// @}
}
#endif
