/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
║                        Query.h - description                                 ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Wednesday November 2 2005                              ║
║  copyright            (C) 1999-2018 by Alexey Parshin. All rights reserved.  ║
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

#ifndef __SPTK_QUERY_H__
#define __SPTK_QUERY_H__

#ifdef WIN32
#include <winsock2.h>
#include <windows.h>
#endif

#include <sptk5/DataSource.h>

#include <sptk5/db/AutoDatabaseConnection.h>
#include <sptk5/db/QueryParameterList.h>
#include <sptk5/FieldList.h>
#include <sptk5/threads/Locks.h>

namespace sptk {

/**
 * @addtogroup Database Database Support
 * @{
 */

/**
 * @brief Database query
 *
 * A CDataset connected to the database to
 * execute a database queries. The type of the database
 * depends on the DatabaseConnection object query is connected to.
 */
class SP_EXPORT Query: public DataSource, protected SharedMutex
{
    friend class PoolDatabaseConnection;

    /**
     * Prepare the query automatically, on thedynamic_cast<COracleBulkInsertQuery*>( first call
     */
    bool                    m_autoPrepare {false};

    /**
     * ODBC statement handle
     */
    void*                   m_statement {nullptr};

    /**
     * Database server connection
     */
    void*                   m_connection {nullptr};

    /**
     * True if the statement is prepared
     */
    bool                    m_prepared {false};

    /**
     * True if query is active (opened)
     */
    bool                    m_active {false};

    /**
     * True if there is no more records to fetch
     */
    bool                    m_eof {true};

    /**
     * List of query parameters
     */
    QueryParameterList      m_params;

    /**
     * List of query fields - makes sense after fetch
     */
    FieldList               m_fields;

    /**
     * Database connection
     */
    PoolDatabaseConnection* m_db {nullptr};

    /**
     * SQL statement string
     */
    String                  m_sql;

    /**
     * Optional diagnostic messages populated after exec() or open()
     */
    Strings                 m_messages;

    /**
     * Unique index for query object
     */
    int                     m_objectIndex {0};

protected:

    /**
     * The source file the query was created in
     */
    const char*             m_createdFile {nullptr};

    /**
     * The source file line the query was created at
     */
    unsigned                m_createdLine {0};


    /**
     * Bulk mode flag
     */
    bool                    m_bulkMode {false};

    /**
     * Counts columns of the dataset (if any) returned by query
     */
    int countCols();

    /**
     * Allocates a statement
     */
    void allocStmt();

    /**
     * Deallocates a statement. All the resources allocated by statement are released.
     */
    void freeStmt();

    /**
     * Closes a statement. Prepared statement stay prepared but closed.
     */
    void closeStmt();

    /**
     * Executes a statement
     */
    void execute();

    /**
     * In a dataset returned by a query, retrieves the column attributes
     */
    void colAttributes(int16_t column, int16_t descType, int32_t& value);

    /**
     * In a dataset returned by a query, retrieves the column attributes
     */
    void colAttributes(int16_t column, int16_t descType, char *buff, int len);


    /**
     * Retrieves an error (if any) after executing an ODBC statement
     */
    String getError() const;

    /**
     * Internal number to implement unique query index. That is pretty useful for logs
     */
    static int      nextObjectIndex;


    /**
     * Internal function to throw 'Not implemented' exception
     */
    void notImplemented(const String& functionName) const;

    /**
     * @brief Closes query by closing the statement.
     *
     * If the statement isn't released it may be re-used later.
     * @param releaseStatement  True if we need to release the query's ODBC statement
     */
    void closeQuery(bool releaseStatement = false);

    /**
     * @brief In CDataset it should load data into the dataset.
     *
     * Since the query loads all the data by open() or fetch(),
     * in Query this method does exactly nothing
     */
    virtual bool loadData()
    {
        return false;
    }

    /**
     * @brief In CDataset it should save data into the dataset.
     *
     * Since the query saves all the data by execute,
     * in Query this method does exactly nothing
     */
    virtual bool saveData()
    {
        return false;
    }

public:
    /**
     * @brief Default constructor
     */
    Query() noexcept;

    /**
     * @brief Constructor
     *
     * You can optionally provide the name of the file and line number where
     * this query is created. This is used to collect statistical information
     * for the query calls. If file and line information is provided, then
     * calls statistics is stored to the database object during the query dtor.
     * @param db                The database to connect to, optional
     * @param sql               The SQL query text to use, optional
     * @param autoPrepare       If true then statement is auto-prepared before execution (if not yet prepared), otherwise it's called directly. Parameter binding is not available in not prepared statements.
     * @param createdFile       The name of the file this query was created in (optional)
     * @param createdLine       The line of the file this query was created at (optional)
     */
    Query(DatabaseConnection db, const String& sql = "", bool autoPrepare = true, const char* createdFile = nullptr, unsigned createdLine = 0);

    /**
     * @brief Constructor
     *
     * You can optionally provide the name of the file and line number where
     * this query is created. This is used to collect statistical information
     * for the query calls. If file and line information is provided, then
     * calls statistics is stored to the database object during the query dtor.
     * @param db                The database to connect to, optional
     * @param sql               The SQL query text to use, optional
     * @param autoPrepare       If true then statement is auto-prepared before execution (if not yet prepared), otherwise it's called directly. Parameter binding is not available in not prepared statements.
     * @param createdFile       The name of the file this query was created in (optional)
     * @param createdLine       The line of the file this query was created at (optional)
     */
    Query(PoolDatabaseConnection *db, const String& sql = "", bool autoPrepare = true, const char* createdFile = nullptr, unsigned createdLine = 0);

    /**
     * @brief Copy constructor
     */
    Query(const Query&);

    /**
     * @brief Destructor
     */
    ~Query();

    /**
     * @brief Finds a field by the field name
     * @param fname const char *, field name
     * @returns CField pointer, or 0L if not found
     */
    Field* fieldByName(const char * fname) const
    {
        return m_fields.fieldByName(fname);
    }

    /**
     * @brief Field access by number, const version
     *
     * Field index should be inside 0..fieldCount()-1
     * @param fieldIndex int, field index
     */
    virtual const Field& operator [](uint32_t fieldIndex) const
    {
        return m_fields[fieldIndex];
    }

    /**
     * @brief Field access by number, const version
     *
     * Field index should be inside 0..fieldCount()-1
     * @param fieldIndex int, field index
     */
    virtual Field& operator [](uint32_t fieldIndex)
    {
        return m_fields[fieldIndex];
    }

    /**
     * @brief Field access by field name, const version
     */
    virtual const Field& operator [](const char *fieldName) const
    {
        return m_fields[fieldName];
    }

    /**
     * @brief Field access by field name.
     */
    virtual Field& operator [](const char *fieldName)
    {
        return m_fields[fieldName];
    }

    /**
     * @brief Field access by field name, const version
     */
    virtual const Field& operator [](const String& fieldName) const
    {
        return m_fields[fieldName.c_str()];
    }

    /**
     * @brief Field access by field name.
     */
    virtual Field& operator [](const String& fieldName)
    {
        return m_fields[fieldName.c_str()];
    }

    /**
     * @brief Reports a number of columns in the recordset for the active query.
     *
     * Typically is used after the open() method is called, but before query is closed.
     */
    virtual uint32_t fieldCount() const
    {
        return m_fields.size();
    }

    /**
     * @brief Reports the record count for the recordset, returned by the open() method.
     *
     * Currently is NOT implemented.
     */
    virtual uint32_t recordCount() const
    {
        notImplemented("recordCount");
        return 0;
    }

    /**
     * @brief Returns the text of current SQL query as String
     */
    String sql()
    {
        return m_sql;
    }

    /**
     * @brief Returns the query fields list
     */
    FieldList& fields()
    {
        return m_fields;
    }

    /**
     * @brief Returns the query parameters list
     */
    QueryParameterList& params()
    {
        return m_params;
    }

    /**
     * @brief Field read access by the field name, for the universal data connection
     */
    virtual bool readField(const char *fname, Variant& value);

    /**
     * @brief Field write access by the field name, for the universal data connection
     */
    virtual bool writeField(const char *fname, const Variant& fvalue);

    /**
     * @brief Opens the query and fetches the first row.
     *
     * It is necessary for the select queries and some stored procedures
     * that may return the dataset. First-time call for open() also prepares the query.
     */
    virtual bool open();

    /**
     * @brief Closes the query
     *
     * Doesn't release the db statement, so the query may be called again, and faster than for the first time.
     */
    virtual bool close()
    {
        closeQuery();
        return true;
    }

    /**
     * @brief Fetches the next row from the recordset, same as fetch()
     */
    virtual bool next()
    {
        fetch();
        return true;
    }

    /**
     * @brief Returns true if there is no more rows in the recordset
     */
    virtual bool eof() const
    {
        return m_eof;
    }

    /**
     * @brief Executes the query and closes the statement.
     */
    virtual void exec()
    {
        open();
    }

    /**
     * @brief Executes the query and closes the statement.
     *
     * Query SQL would be set to the new SQL statement
     * @param newSQL            SQL statement to execute
     */
    virtual void exec(const String& newSQL)
    {
        sql(newSQL);
        open();
    }

    /**
     * @brief Fetches the next row from the recordset, same as next()
     */
    void fetch();

    /**
     * @brief Connects a query to a database
     *
     * If the query was connected
     * to another database, releases all the allocated resources in it.
     */
    void connect(PoolDatabaseConnection *db);

    /**
     * @brief Disconnects query from the database and releases all the allocated resourses.
     */
    void disconnect();

    /**
     * @brief Reports the number of unique parameters in the query.
     *
     * Makes sense after the SQL query text is set.
     */
    uint32_t paramCount() const
    {
        return m_params.size();
    }

    /**
     * @brief Returns the parameter by the name.
     *
     * If the parameter name isn't found, throws an exception
     * @param paramName const char *, parameter name
     * @returns parameter
     * @see CParamList
     */
    QueryParameter& param(const char *paramName) const
    {
        return m_params[paramName];
    }

    /**
     * @brief Returns the parameter by the name.
     *
     * If the parameter name isn't found, throws an exception
     * @param paramName         Parameter name
     * @returns parameter
     * @see CParamList
     */
    QueryParameter& param(const String& paramName) const
    {
        return m_params[paramName.c_str()];
    }

    /**
     * @brief Returns the parameter by the index.
     * @returns parameter
     * @see CParamList
     */
    QueryParameter& param(uint32_t paramIndex) const
    {
        return m_params[int32_t(paramIndex)];
    }

    /**
     * @brief Returns query statement handle
     */
    void *statement() const
    {
        return m_statement;
    }

    /**
     * @brief Returns SQL Query text
     */
    String sql() const
    {
        return m_sql;
    }

    /**
     * @brief Sets SQL Query text.
     * If the Query text is not the same and the db statement was prepared earlier
     * then the db statement is released and new one is created.
     */
    virtual void sql(const String& _sql);

    /**
     * @brief Returns the database the query is connected to
     */
    PoolDatabaseConnection *database() const
    {
        return m_db;
    }

    /**
     * @brief Connects the query to the database different database.
     */
    void database(PoolDatabaseConnection *db)
    {
        connect(db);
    }

    /**
     * @brief Reports if the query is opened
     */
    bool active() const
    {
        return m_active;
    }

    /**
     * @brief True if the statement is prepared
     */
    bool prepared() const
    {
        return m_prepared;
    }

    /**
     * @brief Returns the value for auto-prepare flag.
     *
     * If the flag is set the query would automatically call prepare() when needed.
     */
    bool autoPrepare() const
    {
        return m_autoPrepare;
    }

    /**
     * @brief Sets the value for auto-prepare flag.
     *
     * If the flag is set the query would automatically call prepare() when needed.
     */
    void autoPrepare(bool ap)
    {
        m_autoPrepare = ap;
    }

    /**
     * @brief Prepares query for the fast execution
     */
    virtual void prepare();

    /**
     * @brief Unprepares query releasing previously prepared statement
     */
    virtual void unprepare();

    /**
     * @brief Throws an exception
     *
     * Before exception is thrown, it is logged into the logfile (if the logfile is defined)
     * @param method            Method name where error has occured
     * @param error             Error text
     */
    void throwError(const String& method, const String& error);

    /**
     * @brief Access to diag messages
     *
     * Some of the database drivers (ODBC, for example) may return diag messages
     * after the execution of the query. Usually, such messages may be generated
     * by a stored procedure call.
     */
    Strings& messages()
    {
        return m_messages;
    }

    /**
     *@ brief Returns bulk mode flag
     */
    bool bulkMode() const { return m_bulkMode; }
};
/**
 * @}
 */

#define THROW_QUERY_ERROR(query, error) { std::stringstream err; err << error; throw sptk::DatabaseException(err.str(),__FILE__,__LINE__, query->sql()); }

}
#define FETCH_BUFFER_SIZE 1024

#endif
