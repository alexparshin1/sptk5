/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2021 Alexey Parshin. All rights reserved.       ║
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

#include <sptk5/cutils>
#include <sptk5/db/MySQLConnection.h>
#include <sptk5/db/Query.h>

using namespace std;
using namespace sptk;

MySQLConnection::MySQLConnection(const String& connectionString)
: PoolDatabaseConnection(connectionString, DatabaseConnectionType::MYSQL)
{
}

MySQLConnection::~MySQLConnection()
{
    try {
        if (getInTransaction() && MySQLConnection::active())
            rollbackTransaction();
        disconnectAllQueries();
        close();
    } catch (const Exception& e) {
        CERR(e.what() << endl)
    }
}

void MySQLConnection::initConnection()
{
    static std::mutex libraryInitMutex;

    lock_guard<mutex> lock(libraryInitMutex);
    m_connection = mysql_init(m_connection);
    if (m_connection == nullptr)
        throw DatabaseException("Can't initialize MySQL environment");
    mysql_options(m_connection, MYSQL_SET_CHARSET_NAME, "utf8");
    mysql_options(m_connection, MYSQL_INIT_COMMAND, "SET NAMES utf8");
}

void MySQLConnection::_openDatabase(const String& newConnectionString)
{
    if (!active()) {
        setInTransaction(false);
        if (!newConnectionString.empty())
            connectionString(DatabaseConnectionString(newConnectionString));

        initConnection();

        string connectionError;
        const DatabaseConnectionString& connString = connectionString();
        if (mysql_real_connect(m_connection,
                               connString.hostName().c_str(),
                               connString.userName().c_str(),
                               connString.password().c_str(),
                               connString.databaseName().c_str(),
                               connString.portNumber(),
                               nullptr,
                               CLIENT_MULTI_RESULTS) == nullptr)
        {
            connectionError = mysql_error(m_connection);
            mysql_close(m_connection);
            m_connection = nullptr;
            throw DatabaseException("Can't connect to MySQL: " + connectionError);
        }
    }
}

void MySQLConnection::closeDatabase()
{
    disconnectAllQueries();

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

void MySQLConnection::executeCommand(const String& command)
{
    if (m_connection == nullptr)
        open();

    if (mysql_real_query(m_connection, command.c_str(), ULONG_CAST(command.length())) != 0)
        throwMySQLException("Can't execute " + command);
}

void MySQLConnection::driverBeginTransaction()
{
    if (getInTransaction())
        throwMySQLException("Transaction already started");

    executeCommand("BEGIN");
    setInTransaction(true);
}

void MySQLConnection::driverEndTransaction(bool commit)
{
    if (!getInTransaction()) throwDatabaseException("Transaction isn't started.")

    const char* action = commit ? "COMMIT" : "ROLLBACK";
    executeCommand(action);
    setInTransaction(false);
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
    auto* statement = (MySQLStatement*) query->statement();
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
        auto* statement = (MySQLStatement*) query->statement();
        if (statement != nullptr)
            statement->close();
    }
    catch (const Exception& e) {
        THROW_QUERY_ERROR(query, e.what())
    }
}

void MySQLConnection::queryPrepare(Query* query)
{
    lock_guard<mutex> lock(m_mutex);

    if (!query->prepared()) {
        auto* statement = (MySQLStatement*) query->statement();
        if (statement != nullptr) {
            try {
                statement->prepare(query->sql());
                statement->enumerateParams(query->params());
                querySetPrepared(query, true);
            }
            catch (const Exception& e) {
                THROW_QUERY_ERROR(query, e.what())
            }
        }
    }
}

void MySQLConnection::queryUnprepare(Query* query)
{
    queryFreeStmt(query);
    querySetPrepared(query, false);
}

int MySQLConnection::queryColCount(Query* query)
{
    int colCount = 0;
    const auto* statement = (MySQLStatement*) query->statement();
    try {
        if (statement == nullptr)
            throw DatabaseException("Query not opened");
        colCount = (int) statement->colCount();
    }
    catch (const Exception& e) {
        THROW_QUERY_ERROR(query, e.what())
    }
    return colCount;
}

void MySQLConnection::queryBindParameters(Query* query)
{
    lock_guard<mutex> lock(m_mutex);

    auto* statement = (MySQLStatement*) query->statement();
    try {
        if (statement == nullptr)
            throw DatabaseException("Query not prepared");
        statement->setParameterValues();
    }
    catch (const Exception& e) {
        THROW_QUERY_ERROR(query, e.what())
    }
}

void MySQLConnection::queryExecute(Query* query)
{
    auto* statement = (MySQLStatement*) query->statement();
    try {
        if (statement == nullptr)
            throw DatabaseException("Query is not prepared");
        statement->execute(getInTransaction());
    }
    catch (const Exception& e) {
        THROW_QUERY_ERROR(query, e.what())
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
        if (!query->prepared()) {
            queryPrepare(query);
        }
        queryBindParameters(query);
    }

    auto* statement = (MySQLStatement*) query->statement();

    queryExecute(query);
    auto fieldCount = (short) queryColCount(query);
    if (fieldCount < 1)
        return;

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
    if (!query->active())
        THROW_QUERY_ERROR(query, "Dataset isn't open")

    lock_guard<mutex> lock(m_mutex);

    try {
        auto* statement = (MySQLStatement*) query->statement();

        statement->fetch();
        if (statement->eof()) {
            querySetEof(query, true);
            return;
        }

        statement->readResultRow(query->fields());
    }
    catch (const Exception& e) {
        Query::throwError("CMySQLConnection::queryFetch", e.what());
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
        default:
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
    catch (const Exception& e) {
        CERR("Error fetching system info: " << e.what() << endl)
    }
}

void MySQLConnection::_executeBatchSQL(const Strings& sqlBatch, Strings* errors)
{
    shared_ptr<RegularExpression> matchStatementEnd(new RegularExpression("(;\\s*)$"));

    RegularExpression matchDelimiterChange("^DELIMITER\\s+(\\S+)");
    RegularExpression matchEscapeChars("([$.])", "g");
    RegularExpression matchCommentRow("^\\s*--");

    Strings statements;
    String statement;
    for (auto row: sqlBatch) {
        row = row.trim();
        if (row.empty() || matchCommentRow.matches(row))
            continue;
        auto matches = matchDelimiterChange.m(row);
        if (matches) {
            auto delimiter = matches[0].value;
            delimiter = matchEscapeChars.s(delimiter, "\\\\1");
            matchStatementEnd = make_shared<RegularExpression>("(" + delimiter + ")(\\s*|-- .*)$");
            statement = "";
            continue;
        }
        if (matchStatementEnd->matches(row)) {
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

    for (const auto& stmt: statements) {
        Query query(this, stmt, false);
        try {
            query.exec();
        }
        catch (const Exception& e) {
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
    auto* connection = new MySQLConnection(connectionString);
    return connection;
}

void mysql_destroy_connection(void* connection)
{
    delete (MySQLConnection*) connection;
}
