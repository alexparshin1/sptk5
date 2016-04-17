/***************************************************************************
                          SIMPLY POWERFUL TOOLKIT (SPTK)
                          CQuery.h  -  description
                             -------------------
    begin                : Wed Dec 15 1999
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

#ifndef __CQUERY_H__
#define __CQUERY_H__

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#include <sptk5/CDataSource.h>

#include <sptk5/db/CDatabaseConnection.h>
#include <sptk5/db/CParamList.h>
#include <sptk5/CFieldList.h>

namespace sptk {

/// @addtogroup Database Database Support
/// @{

/// @brief Database query
///
/// A CDataset connected to the database to
/// execute a database queries. The type of the database
/// depends on the CDatabaseConnection object query is connected to.
class SP_EXPORT CQuery: public CDataSource, public CSynchronized
{
    friend class CDatabaseConnection;

protected:
    bool            m_autoPrepare;     ///< Prepare the query automatically, on thedynamic_cast<COracleBulkInsertQuery*>( first call
    void*           m_statement;       ///< ODBC statement handle
    void*           m_connection;      ///< Database server connection

    bool            m_prepared;        ///< True if the statement is prepared
    bool            m_active;          ///< True if query is active (opened)
    bool            m_eof;             ///< True if there is no more records to fetch
    CParamList      m_params;          ///< List of query parameters
    CFieldList      m_fields;          ///< List of query fields - makes sense after fetch

    double          m_duration;        ///< The duration of last execution, in seconds
    double          m_totalDuration;   ///< The total duration of executions, in seconds
    unsigned        m_totalCalls;      ///< The total number of query executions

    CDatabaseConnection*m_db;          ///< Database connection
    std::string     m_sql;             ///< SQL statement string
    const char*     m_createdFile;     ///< The source file the query was created in
    unsigned        m_createdLine;     ///< The source file line the query was created at

    CStrings        m_messages;        ///< Optional diag messages populated after exec() or open()
    bool            m_bulkMode;        ///< Bulk mode flag

    int countCols();                   ///< Counts columns of the dataset (if any) returned by query
    void allocStmt();                  ///< Allocates a statement
    void freeStmt();                   ///< Deallocates a statement. All the resources allocated by statement are released.
    void closeStmt();                  ///< Closes a statement. Prepared statement stay prepared but closed.
    void execute();                    ///< Executes a statement
    void colAttributes(int16_t column, int16_t descType, int32_t& value); ///< In a dataset returned by a query, retrieves the column attributes
    void colAttributes(int16_t column, int16_t descType, char *buff, int len); ///< In a dataset returned by a query, retrieves the column attributes
    std::string getError() const;      ///< Retrieves an error (if any) after executing an ODBC statement

    int             m_objectIndex;     ///< Unique index for query object
    static int      nextObjectIndex;   ///< Internal number to implement unique query index. That is pretty useful for logs

    void notImplemented(std::string functionName) const; ///< Internal function to throw 'Not implemented' exception

protected:
    /// @brief Closes query by closing the statement.
    ///
    /// If the statement isn't released it may be re-used later.
    /// @param releaseStatement bool, true if we need to release the query's ODBC statement
    void closeQuery(bool releaseStatement = false);

    /// @brief In CDataset it should load data into the dataset.
    ///
    /// Since the query loads all the data by open() or fetch(),
    /// in CQuery this method does exactly nothing
    virtual bool loadData()
    {
        return false;
    }

    /// @brief In CDataset it should save data into the dataset.
    ///
    /// Since the query saves all the data by execute,
    /// in CQuery this method does exactly nothing
    virtual bool saveData()
    {
        return false;
    }

    /// @brief Stores calls statistics to the database
    void storeStatistics();

public:
    /// @brief Constructor
    ///
    /// You can optionally provide the name of the file and line number where
    /// this query is created. This is used to collect statistical information
    /// for the query calls. If file and line information is provided, then
    /// calls statistics is stored to the database object during the query dtor.
    /// @param db CDatabaseConnection, the database to connect to, optional
    /// @param sql std::string, the SQL query text to use, optional
    /// @param autoPrepare bool, if true then statement is auto-prepared before execution (if not yet prepared), otherwise it's called directly. Parameter binding is not available in not prepared statements.
    /// @param createdFile const char*, the name of the file this query was created in (optional)
    /// @param createdLine unsigned, the line of the file this query was created at (optional)
    CQuery(CDatabaseConnection *db = 0L, std::string sql = "", bool autoPrepare = true, const char* createdFile = 0, unsigned createdLine = 0);

    /// @brief Copy constructor
    CQuery(const CQuery&);

    /// @brief Destructor
    ~CQuery();

    /// @brief Finds a field by the field name
    /// @param fname const char *, field name
    /// @returns CField pointer, or 0L if not found
    CField* fieldByName(const char * fname) const
    {
        return m_fields.fieldByName(fname);
    }

    /// @brief Field access by number, const version
    ///
    /// Field index should be inside 0..fieldCount()-1
    /// @param fieldIndex int, field index
    virtual const CField& operator [](uint32_t fieldIndex) const
    {
        return m_fields[fieldIndex];
    }

    /// @brief Field access by number, const version
    ///
    /// Field index should be inside 0..fieldCount()-1
    /// @param fieldIndex int, field index
    virtual CField& operator [](uint32_t fieldIndex)
    {
        return m_fields[fieldIndex];
    }

    /// @brief Field access by field name, const version
    virtual const CField& operator [](const char *fieldName) const
    {
        return m_fields[fieldName];
    }

    /// @brief Field access by field name.
    virtual CField& operator [](const char *fieldName)
    {
        return m_fields[fieldName];
    }

    /// @brief Field access by field name, const version
    virtual const CField& operator [](const std::string& fieldName) const
    {
        return m_fields[fieldName.c_str()];
    }

    /// @brief Field access by field name.
    virtual CField& operator [](const std::string& fieldName)
    {
        return m_fields[fieldName.c_str()];
    }

    /// @brief Reports a number of columns in the recordset for the active query.
    ///
    /// Typically is used after the open() method is called, but before query is closed.
    virtual uint32_t fieldCount() const
    {
        return m_fields.size();
    }

    /// @brief Reports the record count for the recordset, returned by the open() method.
    ///
    /// Currently is NOT implemented.
    virtual uint32_t recordCount() const
    {
        notImplemented("recordCount");
        return 0;
    }

    /// @brief Returns the text of current SQL query as std::string
    std::string sql()
    {
        return m_sql;
    }

    /// @brief Returns the query fields list
    CFieldList& fields()
    {
        return m_fields;
    }

    /// @brief Returns the query parameters list
    CParamList& params()
    {
        return m_params;
    }

public:

    /// @brief Field read access by the field name, for the universal data connection
    virtual bool readField(const char *fname, CVariant& value);

    /// @brief Field write access by the field name, for the universal data connection
    virtual bool writeField(const char *fname, const CVariant& fvalue);

public:
    /// @brief Opens the query and fetches the first row.
    ///
    /// It is necessary for the select queries and some stored procedures
    /// that may return the dataset. First-time call for open() also prepares the query.
    virtual bool open() THROWS_EXCEPTIONS;

    /// @brief Closes the query
    ///
    /// Doesn't release the db statement, so the query may be called again, and faster than for the first time.
    virtual bool close()
    {
        closeQuery();
        return true;
    }

    /// @brief Fetches the next row from the recordset, same as fetch()
    virtual bool next()
    {
        fetch();
        return true;
    }

    /// @brief Returns true if there is no more rows in the recordset
    virtual bool eof() const
    {
        return m_eof;
    }

public:
    /// @brief Executes the query and closes the statement.
    virtual void exec() THROWS_EXCEPTIONS
    {
        open();
    }

    /// @brief Executes the query and closes the statement.
    ///
    /// Query SQL would be set to the new SQL statement
    /// @param newSQL std::string, an SQL statement to execute
    virtual void exec(std::string newSQL) THROWS_EXCEPTIONS
    {
        sql(newSQL);
        open();
    }

    /// @brief Fetches the next row from the recordset, same as next()
    void fetch() THROWS_EXCEPTIONS;

    /// @brief Connects a query to a database
    ///
    /// If the query was connected
    /// to another database, releases all the allocated resources in it.
    void connect(CDatabaseConnection *db);

    /// @brief Disconnects query from the database and releases all the allocated resourses.
    void disconnect();

    /// @brief Reports the number of unique parameters in the query.
    ///
    /// Makes sense after the SQL query text is set.
    uint32_t paramCount() const
    {
        return m_params.size();
    }

    /// @brief Returns the parameter by the name.
    ///
    /// If the parameter name isn't found, throws an exception
    /// @param paramName const char *, parameter name
    /// @returns parameter
    /// @see CParamList
    CParam& param(const char *paramName) const
    {
        return m_params[paramName];
    }

    /// @brief Returns the parameter by the name.
    ///
    /// If the parameter name isn't found, throws an exception
    /// @param paramName std::string, parameter name
    /// @returns parameter
    /// @see CParamList
    CParam& param(const std::string& paramName) const
    {
        return m_params[paramName.c_str()];
    }

    /// @brief Returns the parameter by the index.
    /// @returns parameter
    /// @see CParamList
    CParam& param(uint32_t paramIndex) const
    {
        return m_params[int32_t(paramIndex)];
    }

    /// @brief Returns query statement handle
    void *statement() const
    {
        return m_statement;
    }

    /// @brief Returns SQL Query text
    std::string sql() const
    {
        return m_sql;
    }

    /// @brief Sets SQL Query text.
    /// If the Query text is not the same and the db statement was prepared earlier
    /// then the db statement is released and new one is created.
    virtual void sql(std::string _sql);

    /// @brief Returns the database the query is connected to
    CDatabaseConnection *database() const
    {
        return m_db;
    }

    /// @brief Connects the query to the database different database.
    void database(CDatabaseConnection *db)
    {
        connect(db);
    }

    /// @brief Reports if the query is opened
    bool active() const
    {
        return m_active;
    }

    /// @brief True if the statement is prepared
    bool prepared() const
    {
        return m_prepared;
    }

    /// @brief Returns the value for auto-prepare flag.
    ///
    /// If the flag is set the query would automatically call prepare() when needed.
    bool autoPrepare() const
    {
        return m_autoPrepare;
    }

    /// @brief Sets the value for auto-prepare flag.
    ///
    /// If the flag is set the query would automatically call prepare() when needed.
    void autoPrepare(bool ap)
    {
        m_autoPrepare = ap;
    }

    /// @brief Prepares query for the fast execution
    virtual void prepare();

    /// @brief Unprepares query releasing previously prepared statement
    virtual void unprepare();

    /// @brief Adds the text to log file
    ///
    /// The log file should be set to active, otherwise no text is added to the log file.
    /// Every successful call of this method adds a new line to the log file.
    /// @param text std::string, log text
    /// @param logPriority const CLogPriority&, log message priority
    void logText(std::string text, const CLogPriority& logPriority = CLP_DEBUG);

    /// @brief Throws an exception
    ///
    /// Before exception is thrown, it is logged into the logfile (if the logfile is defined)
    /// @param method std::string, method name where error has occured
    /// @param error std::string, error text
    void logAndThrow(std::string method, std::string error) THROWS_EXCEPTIONS;

    /// @brief Access to diag messages
    ///
    /// Some of the database drivers (ODBC, for example) may return diag messages
    /// after the execution of the query. Usually, such messages may be generated
    /// by a stored procedure call.
    CStrings& messages()
    {
        return m_messages;
    }

    ///@ brief Returns bulk mode flag
    bool bulkMode() const { return m_bulkMode; }
};
/// @}
}
#define FETCH_BUFFER_SIZE 1024

#endif
