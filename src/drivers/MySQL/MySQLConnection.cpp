/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
║                       CMySQLConnection.cpp - description                     ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  begin                Thursday May 25 2000                                   ║
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

#include <sptk5/RegularExpression.h>
#include <sptk5/db/MySQLConnection.h>
#include <sptk5/db/Query.h>

using namespace std;
using namespace sptk;

MySQLConnection::MySQLConnection(const String& connectionString)
: PoolDatabaseConnection(connectionString),
  m_connection(nullptr)
{
    m_connType = DCT_MYSQL;
}

MySQLConnection::~MySQLConnection()
{
    try {
        if (m_inTransaction && active())
            rollbackTransaction();
        close();
        while (!m_queryList.empty()) {
            try {
                auto query = (Query*) m_queryList[0];
                query->disconnect();
            } catch (...) {
            }
        }
        m_queryList.clear();
    } catch (...) {
    }
}


void MySQLConnection::_openDatabase(const String& newConnectionString)
{
    static std::mutex libraryInitMutex;

    if (!active()) {
        m_inTransaction = false;
        if (!newConnectionString.empty())
            m_connString = DatabaseConnectionString(newConnectionString);

        {
            lock_guard<mutex> lock(libraryInitMutex);
            m_connection = mysql_init(m_connection);
        }

        mysql_options(m_connection, MYSQL_SET_CHARSET_NAME, "utf8");
        mysql_options(m_connection, MYSQL_INIT_COMMAND, "SET NAMES utf8");

        string connectionError;
        if (m_connection == nullptr)
            connectionError = "Can't initialize MySQL environment";
        else {
            if (mysql_real_connect(m_connection,
                                   m_connString.hostName().c_str(),
                                   m_connString.userName().c_str(),
                                   m_connString.password().c_str(),
                                   m_connString.databaseName().c_str(),
                                   m_connString.portNumber(),
                                   nullptr,
                                   CLIENT_MULTI_RESULTS) == nullptr) {
                connectionError = mysql_error(m_connection);
                mysql_close(m_connection);
                m_connection = nullptr;
            }
        }

        if (m_connection == nullptr)
            throw DatabaseException("Can't connect to MySQL: " + connectionError);
    }
}

void MySQLConnection::closeDatabase()
{
    for (auto query: m_queryList) {
        try {
            queryFreeStmt(query);
        } catch (...) {
        }
    }

    if (m_connection != nullptr) {
        mysql_close(m_connection);
        m_connection = nullptr;
    }
}

void* MySQLConnection::handle() const
{
    return m_connection;
}

bool MySQLConnection::active() const
{
    return m_connection != nullptr;
}

String MySQLConnection::nativeConnectionString() const
{
    // Connection string in format: host[:port][/instance]
    stringstream connectionString(m_connString.hostName());
    if (m_connString.portNumber() != 0)
        connectionString << ":" << int2string(m_connString.portNumber());
    if (!m_connString.databaseName().empty())
        connectionString << "/" << m_connString.databaseName();
    return connectionString.str();
}

void MySQLConnection::executeCommand(const String& command)
{
    if (m_connection == nullptr)
        open();

    if (mysql_real_query(m_connection, command.c_str(), command.length()) != 0)
        throwMySQLException("Can't execute " + command);
}

void MySQLConnection::driverBeginTransaction()
{
    if (m_inTransaction)
        throwMySQLException("Transaction already started");

    executeCommand("BEGIN");
    m_inTransaction = true;
}

void MySQLConnection::driverEndTransaction(bool commit)
{
    if (!m_inTransaction) throwDatabaseException("Transaction isn't started.");

    const char* action = commit ? "COMMIT" : "ROLLBACK";
    executeCommand(action);
    m_inTransaction = false;
}

//-----------------------------------------------------------------------------------------------
String MySQLConnection::queryError(const Query*) const
{
    return mysql_error(m_connection);
}

void MySQLConnection::queryAllocStmt(Query* query)
{
    queryFreeStmt(query);
    querySetStmt(query, new MySQLStatement(this, query->sql(), query->autoPrepare()));
}

void MySQLConnection::queryFreeStmt(Query* query)
{
    lock_guard<mutex> lock(m_mutex);
    auto statement = (MySQLStatement*) query->statement();
    if (statement != nullptr) {
        delete statement;
        querySetStmt(query, nullptr);
        querySetPrepared(query, false);
    }
}

void MySQLConnection::queryCloseStmt(Query* query)
{
    lock_guard<mutex> lock(m_mutex);
    try {
        auto statement = (MySQLStatement*) query->statement();
        if (statement != nullptr)
            statement->close();
    }
    catch (exception& e) {
        THROW_QUERY_ERROR(query, e.what());
    }
}

void MySQLConnection::queryPrepare(Query* query)
{
    lock_guard<mutex> lock(m_mutex);

    if (!query->prepared()) {
        auto statement = (MySQLStatement*) query->statement();
        if (statement != nullptr) {
            try {
                statement->prepare(query->sql());
                statement->enumerateParams(query->params());
            }
            catch (exception& e) {
                THROW_QUERY_ERROR(query, e.what());
            }
            //querySetPrepared(query, true);
        }
    }
}

void MySQLConnection::queryUnprepare(Query* query)
{
    queryFreeStmt(query);
}

int MySQLConnection::queryColCount(Query* query)
{
    int colCount = 0;
    auto statement = (MySQLStatement*) query->statement();
    try {
        if (statement == nullptr) throwDatabaseException("Query not opened");
        colCount = (int) statement->colCount();
    }
    catch (exception& e) {
        THROW_QUERY_ERROR(query, e.what());
    }
    return colCount;
}

void MySQLConnection::queryBindParameters(Query* query)
{
    lock_guard<mutex> lock(m_mutex);

    auto statement = (MySQLStatement*) query->statement();
    try {
        if (statement == nullptr)
            throwDatabaseException("Query not prepared");
        statement->setParameterValues();
    }
    catch (exception& e) {
        THROW_QUERY_ERROR(query, e.what());
    }
}

void MySQLConnection::queryExecute(Query* query)
{
    auto statement = (MySQLStatement*) query->statement();
    try {
        if (statement == nullptr) throwDatabaseException("Query is not prepared");
        statement->execute(m_inTransaction);
    }
    catch (exception& e) {
        THROW_QUERY_ERROR(query, e.what());
    }
}

void MySQLConnection::queryOpen(Query* query)
{
    if (!active())
        open();

    if (query->active())
        return;

    if (query->statement() == nullptr) {
        queryAllocStmt(query);
    }

    if (query->autoPrepare()) {
        if (!query->prepared())
            queryPrepare(query);
        queryBindParameters(query);
    }

    auto statement = (MySQLStatement*) query->statement();

    queryExecute(query);
    auto fieldCount = (short) queryColCount(query);
    if (fieldCount < 1) {
        return;
    }

    querySetActive(query, true);
    if (query->fieldCount() == 0) {
        lock_guard<mutex> lock(m_mutex);
        statement->bindResult(query->fields());
    }

    querySetEof(query, statement->eof());

    queryFetch(query);
}

void MySQLConnection::queryFetch(Query* query)
{
    if (!query->active()) THROW_QUERY_ERROR(query, "Dataset isn't open");

    lock_guard<mutex> lock(m_mutex);

    try {
        auto statement = (MySQLStatement*) query->statement();

        statement->fetch();
        if (statement->eof()) {
            querySetEof(query, true);
            return;
        }

        statement->readResultRow(query->fields());
    }
    catch (exception& e) {
        query->throwError("CMySQLConnection::queryFetch", e.what());
    }
}

void MySQLConnection::objectList(DatabaseObjectType objectType, Strings& objects)
{
    string objectsSQL;
    objects.clear();
    switch (objectType) {
        case DOT_PROCEDURES:
            objectsSQL =
                    "SELECT CONCAT(routine_schema, '.', routine_name) object_name "
                    "FROM information_schema.routines "
                    "WHERE routine_type = 'PROCEDURE'";
            break;
        case sptk::DOT_FUNCTIONS:
            objectsSQL =
                    "SELECT CONCAT(routine_schema, '.', routine_name) object_name "
                    "FROM information_schema.routines "
                    "WHERE routine_type = 'FUNCTION'";
            break;
        case DOT_TABLES:
            objectsSQL =
                    "SELECT CONCAT(table_schema, '.', table_name) object_name "
                    "FROM information_schema.tables "
                    "WHERE NOT table_schema IN ('mysql','information_schema')";
            break;
        case DOT_VIEWS:
            objectsSQL =
                    "SELECT CONCAT(table_schema, '.', table_name) object_name "
                    "FROM information_schema.views";
            break;
        case DOT_DATABASES:
            objectsSQL =
                    "SHOW SCHEMAS where `Database` NOT IN ('information_schema','performance_schema','mysql')";
            break;
    }
    Query query(this, objectsSQL);
    try {
        query.open();
        while (!query.eof()) {
            objects.push_back(query[uint32_t(0)].asString());
            query.next();
        }
        query.close();
    }
    catch (exception& e) {
        cerr << "Error fetching system info: " << e.what() << endl;
    }
}

void MySQLConnection::_bulkInsert(const String& tableName, const Strings& columnNames, const Strings& data,
                                  const String& format)
{
    char fileName[256];
    snprintf(fileName, sizeof(fileName), ".bulk.insert.%i.%li", getpid(), (long) time(nullptr));
    data.saveToFile(fileName);
    string sql = "LOAD DATA LOCAL INFILE '" + string(fileName) + "' INTO TABLE " + tableName + " (" +
                 columnNames.asString(",") + ") " + format;

    int rc = mysql_query(m_connection, sql.c_str());
    unlink(fileName);
    if (rc != 0) {
        string error = mysql_error(m_connection);
        throwDatabaseException(error);
    }
}

void MySQLConnection::_executeBatchSQL(const Strings& sqlBatch, Strings* errors)
{
    unique_ptr<RegularExpression> matchStatementEnd(new RegularExpression("(;\\s*)$"));

    RegularExpression matchDelimiterChange("^DELIMITER\\s+(\\S+)");
    RegularExpression matchEscapeChars("([$.])", "g");
    RegularExpression matchCommentRow("^\\s*--");

    Strings statements, matches;
    String statement, delimiter = ";";
    for (auto row: sqlBatch) {
        row = row.trim();
        if (row.empty() || matchCommentRow.matches(row))
            continue;
        if (matchDelimiterChange.m(row, matches)) {
            delimiter = matches[0];
            delimiter = matchEscapeChars.s(delimiter, "\\\\1");
            matchStatementEnd = unique_ptr<RegularExpression>(
                    new RegularExpression("(" + delimiter + ")(\\s*|-- .*)$"));
            statement = "";
            continue;
        }
        if (matchStatementEnd->m(row, matches)) {
            row = matchStatementEnd->s(row, "");
            statement += row;
            statements.push_back(statement);
            statement = "";
            continue;
        }
        statement += row + "\n";
    }

    if (!trim(statement).empty())
        statements.push_back(statement);

    for (auto& stmt: statements) {
        Query query(this, stmt, false);
        try {
            query.exec();
        }
        catch (const exception& e) {
            stringstream error;
            error << e.what() << ", query: " << query.sql();
            if (errors != nullptr)
                errors->push_back(error.str());
            else
                throw DatabaseException(error.str());
        }
    }
}

String MySQLConnection::driverDescription() const
{
    if (m_connection != nullptr)
        return string("MySQL ") + mysql_get_server_info(m_connection);
    return "MySQL";
}

String MySQLConnection::paramMark(unsigned)
{
    return "?";
}

void* mysql_create_connection(const char* connectionString)
{
    MySQLConnection* connection = new MySQLConnection(connectionString);
    return connection;
}

void mysql_destroy_connection(void* connection)
{
    delete (MySQLConnection*) connection;
}
