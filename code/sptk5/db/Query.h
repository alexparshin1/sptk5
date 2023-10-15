/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                        SIMPLY POWERFUL TOOLKIT (SPTK)                        ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2023 Alexey Parshin. All rights reserved.       ║
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

#ifdef WIN32
#include <winsock2.h>

#include <windows.h>
#endif

#include <sptk5/DataSource.h>

#include <sptk5/FieldList.h>
#include <sptk5/db/AutoDatabaseConnection.h>
#include <sptk5/db/QueryParameterList.h>

namespace sptk {

/**
 * @addtogroup Database Database Support
 * @{
 */

class SP_EXPORT QueryStatementManagement
    : public DataSource
{
public:
    /**
     * Constructor
     * @param autoPrepare       If true the auto-prepare statement
     */
    explicit QueryStatementManagement(bool autoPrepare)
        : m_autoPrepare(autoPrepare)
    {
    }

    QueryStatementManagement(const QueryStatementManagement& other) = delete;

    /**
     * Returns query statement handle
     */
    StmtHandle statement() const
    {
        return m_statement;
    }

    /**
     * Returns the value for auto-prepare flag.
     *
     * If the flag is set the query would automatically call prepare() when needed.
     */
    bool autoPrepare() const
    {
        return m_autoPrepare;
    }

    /**
     * Reports if the query is opened
     */
    bool active() const
    {
        return m_active;
    }

    /**
     * True if the statement is prepared
     */
    bool prepared() const
    {
        return m_prepared;
    }

    /**
     * Returns true if there is no more rows in the recordset
     */
    bool eof() const override
    {
        return m_eof;
    }

    /**
     * Return bulk mode flag
     * @return true for bulk mode
     */
    bool bulkMode() const;

    /**
     * Connects a query to a database
     *
     * If the query was connected
     * to another database, releases all the allocated resources in it.
     */
    void connect(PoolDatabaseConnection* db);

    /**
     * Disconnects query from the database and releases all the allocated resourses.
     */
    void disconnect();

    /**
     * Returns the database the query is connected to
     */
    PoolDatabaseConnection* database() const
    {
        return m_db;
    }

    /**
     * Connects the query to the database different database.
     */
    void database(PoolDatabaseConnection* db)
    {
        connect(db);
    }

protected:
    /**
     * Set database (internal)
     */
    void setDatabase(PoolDatabaseConnection* db);

    /**
     * Returns query statement handle
     */
    void setStatement(StmtHandle statement)
    {
        m_statement = statement;
    }

    void setPrepared(bool prepared)
    {
        m_prepared = prepared;
    }

    void setActive(bool active)
    {
        m_active = active;
    }

    void setEof(bool eof)
    {
        m_eof = eof;
    }

    /**
     * Set bulk mode flag
     * @param _bulkMode        True for bulk mode
     */
    void setBulkMode(bool _bulkMode);

    /**
     * Closes a statement. Prepared statement stay prepared but closed.
     * @param freeStatement     If true then statement is freed.
     */
    void closeStmt(bool freeStatement = false);

    /**
     * Closes query by closing the statement.
     *
     * If the statement isn't released it may be re-used later.
     * @param releaseStatement  True if we need to release the query's ODBC statement
     */
    void closeQuery(bool releaseStatement = false);

    /**
     * Optional diagnostic messages populated after exec() or open()
     */
    Strings& messages()
    {
        return m_messages;
    }

    String& getSQL()
    {
        return m_sql;
    }

    const String& getSQL() const
    {
        return m_sql;
    }

    void setSQL(const String& sql)
    {
        m_sql = sql;
    }

    /**
     * Internal function to throw 'Not implemented' exception
     */
    [[noreturn]] void notImplemented(const String& functionName) const;

private:
    bool m_autoPrepare {true};              ///< Prepare the query automatically, on the first call
    StmtHandle m_statement {nullptr};       ///< DB statement handle
    bool m_prepared {false};                ///< True if the statement is prepared
    bool m_active {false};                  ///< True if query is active (opened)
    bool m_eof {true};                      ///< True if there is no more records to fetch
    bool m_bulkMode {false};                ///< Bulk mode flag
    String m_sql;                           ///< SQL statement string
    Strings m_messages;                     ///< Optional diagnostic messages populated after exec() or open()
    PoolDatabaseConnection* m_db {nullptr}; ///< Database connection
};

/**
 * Database query
 *
 * A CDataset connected to the database to
 * execute a database queries. The type of the database
 * depends on the DatabaseConnection object query is connected to.
 */
class SP_EXPORT Query
    : public QueryStatementManagement
{
    friend class PoolDatabaseConnection;

    friend class PoolDatabaseConnectionQueryMethods;

public:
    /**
     * Default constructor
     */
    Query() noexcept;

    /**
     * Constructor
     *
     * You can optionally provide the name of the file and line number where
     * this query is created. This is used to collect statistical information
     * for the query calls. If file and line information is provided, then
     * calls statistics is stored to the database object during the query dtor.
     * @param db                The database to connect to, optional
     * @param sql               The SQL query text to use, optional
     * @param autoPrepare       If true then statement is auto-prepared before execution (if not yet prepared), otherwise it's called directly. Parameter binding is not available in not prepared statements.
     */
    explicit Query(const DatabaseConnection& db, const String& sql = "", bool autoPrepare = true);

    /**
     * Constructor
     *
     * You can optionally provide the name of the file and line number where
     * this query is created. This is used to collect statistical information
     * for the query calls. If file and line information is provided, then
     * calls statistics is stored to the database object during the query dtor.
     * @param db                The database to connect to, optional
     * @param sql               The SQL query text to use, optional
     * @param autoPrepare       If true then statement is auto-prepared before execution (if not yet prepared), otherwise it's called directly. Parameter binding is not available in not prepared statements.
     */
    explicit Query(PoolDatabaseConnection* db, const String& sql = "", bool autoPrepare = true);

    /**
     * Deleted copy constructor
     */
    Query(const Query&) = delete;

    Query& operator=(const Query&) = delete;

    /**
     * Destructor
     */
    ~Query() override;

    /**
     * Field access by number, const version
     *
     * Field index should be inside 0..fieldCount()-1
     * @param fieldIndex        Field index
     */
    Field& operator[](size_t fieldIndex) override
    {
        return m_fields[(int) fieldIndex];
    }

    /**
     * Field access by field name.
     */
    Field& operator[](const String& fieldName) override
    {
        return m_fields[fieldName];
    }

    /**
     * Returns field count in the recordset
     * @returns field count
     */
    size_t fieldCount() const override
    {
        return m_fields.size();
    }

    /**
     * Reports the record count for the recordset, returned by the open() method.
     *
     * Currently is NOT implemented.
     */
    [[noreturn]] size_t recordCount() const override
    {
        notImplemented("recordCount");
    }

    /**
     * Returns the text of current SQL query as String
     */
    [[nodiscard]] virtual String sql() const
    {
        return getSQL();
    }

    /**
     * Sets SQL Query text.
     * If the Query text is not the same and the db statement was prepared earlier
     * then the db statement is released and new one is created.
     */
    virtual void sql(const String& _sql);

    /**
     * Returns the query fields list
     */
    FieldList& fields()
    {
        return m_fields;
    }

    /**
     * Returns the query parameters list
     */
    QueryParameterList& params()
    {
        return m_params;
    }

    /**
     * Field read access by the field name, for the universal data connection
     */
    bool readField(const char* fname, Variant& value) override;

    /**
     * Field write access by the field name, for the universal data connection
     */
    bool writeField(const char* fname, const Variant& fvalue) override;

    /**
     * Opens the query and fetches the first row.
     *
     * It is necessary for the select queries and some stored procedures
     * that may return the dataset. First-time call for open() also prepares the query.
     */
    bool open() override;

    /**
     * Closes the query
     *
     * Doesn't release the db statement, so the query may be called again, and faster than for the first time.
     */
    bool close() override
    {
        closeQuery();
        return true;
    }

    /**
     * Fetches the next row from the recordset, same as fetch()
     */
    bool next() override
    {
        fetch();
        return true;
    }

    /**
     * Executes the query and closes the statement.
     */
    virtual void exec()
    {
        open();
    }

    /**
     * Executes the query and closes the statement.
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
     * Fetches the next row from the recordset, same as next()
     */
    void fetch();

    /**
     * Reports the number of unique parameters in the query.
     *
     * Makes sense after the SQL query text is set.
     */
    size_t paramCount() const
    {
        return m_params.size();
    }

    /**
     * Returns the parameter by the name.
     *
     * If the parameter name isn't found, throws an exception
     * @param paramName const char *, parameter name
     * @returns parameter
     * @see CParamList
     */
    QueryParameter& param(const char* paramName) const
    {
        return m_params[paramName];
    }

    /**
     * Returns the parameter by the name.
     *
     * If the parameter name isn't found, throws an exception
     * @param paramName         Parameter name
     * @returns parameter reference
     */
    QueryParameter& param(const String& paramName) const
    {
        return m_params[paramName.c_str()];
    }

    /**
     * Returns the parameter by the index.
     * @param paramIndex        Parameter index
     * @returns parameter reference
     */
    QueryParameter& param(size_t paramIndex) const
    {
        return m_params[paramIndex];
    }

    /**
     * Throws an exception
     *
     * Before exception is thrown, it is logged into the logfile (if the logfile is defined)
     * @param method            Method name where error has occured
     * @param error             Error text
     */
    [[noreturn]] static void throwError(const String& method, const String& error);

protected:
    /**
     * Executes a statement
     */
    void execute();

    /**
     * In CDataset it should load data into the dataset.
     *
     * Since the query loads all the data by open() or fetch(),
     * in Query this method does exactly nothing
     */
    bool loadData() override
    {
        return false;
    }

    /**
     * In CDataset it should save data into the dataset.
     *
     * Since the query saves all the data by execute,
     * in Query this method does exactly nothing
     */
    bool saveData() override
    {
        return false;
    }

private:
    /**
     * List of query parameters
     */
    QueryParameterList m_params;

    /**
     * List of query fields - makes sense after fetch
     */
    FieldList m_fields {true};

    /**
     * Parse query parameter during assigning SQL to query
     * @param paramStart        Start of parameter
     * @param paramEnd          End of parameter
     * @param paramNumber       Current parameter (placeholder) number
     * @param sql               Current SQL (output)
     */
    void sqlParseParameter(const char* paramStart, const char* paramEnd, int& paramNumber, String& sql);

    String parseParameters(const String& _sql);

    const char* readParameter(String& sql, int& paramNumber, const char* paramStart, const char* paramEnd);
};

using SQuery = std::shared_ptr<Query>;

[[noreturn]] void THROW_QUERY_ERROR(const Query* query, const String& error, std::source_location location = std::source_location::current());

/**
 * @}
 */


constexpr int FETCH_BUFFER_SIZE = 1024;

} // namespace sptk
