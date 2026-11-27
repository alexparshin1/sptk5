/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
║  email                alexeyp@gmail.com                                      ║
║  code review          2026-03-06                                             ║
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
│                                                                              │
│   As a special exception, the copyright holder gives permission to link      │
│   this library with independent modules, whether statically or               │
│   dynamically, and to distribute the resulting work under terms of your      │
│   choice, without any of the additional requirements of section 6 of the     │
│   GNU Library General Public License. An independent module is a module      │
│   which is not derived from or based on this library. If you modify this     │
│   library, you must extend this exception to your version, but you are       │
│   not obliged to do so; if you do not wish to, delete this exception         │
│   statement from your version.                                               │
│                                                                              │
│   Please report all bugs and problems to alexeyp@gmail.com.                  │
└──────────────────────────────────────────────────────────────────────────────┘
*/

#include "sptk5/db/BulkQuery.h"
#include <sptk5/cutils>
#include <sptk5/db/MySQLConnection.h>
#include <sstream> // libc++

using namespace std;
using namespace sptk;

namespace {
[[noreturn]] void throwMySQLException(const shared_ptr<MYSQL>& connection, const String& info)
{
    throw DatabaseException(String(info) + ":" + String(mysql_error(connection.get())));
}
} // namespace

MySQLConnection::MySQLConnection(const String& connectionString, const chrono::seconds connectTimeout)
    : PoolDatabaseConnection(connectionString, DatabaseConnectionType::MYSQL, connectTimeout)
{
}

// Caller must hold m_mutex.
void MySQLConnection::initConnectionUnlocked()
{
    static std::mutex libraryInitMutex;

    const scoped_lock lock(libraryInitMutex);
    m_connection = shared_ptr<MYSQL>(mysql_init(nullptr),
                                     [](auto* connection)
                                     {
                                         mysql_close(connection);
                                     });
    if (m_connection == nullptr)
    {
        throw DatabaseException("Can't initialize MySQL environment");
    }

// The option is an enumerator, not a macro, so the build probes the client headers and defines
// SQL_OPT_SSL_VERIFY_SERVER_CERT only when they provide it. MariaDB and MySQL 5.7 have it; MySQL 8
// and 9 replaced it with MYSQL_OPT_SSL_MODE. Disable verification where the option exists so that
// connections to servers without TLS are allowed.
#ifdef SQL_OPT_SSL_VERIFY_SERVER_CERT
    if (constexpr unsigned int ssl_enforce = 0;
        mysql_options(m_connection.get(), MYSQL_OPT_SSL_VERIFY_SERVER_CERT, &ssl_enforce))
    {
        const string error(mysql_error(m_connection.get()));
        m_connection.reset();
        throw DatabaseException("Can't initialize MySQL environment: " + error);
    }
#endif

    mysql_options(m_connection.get(), MYSQL_SET_CHARSET_NAME, "utf8");
    mysql_options(m_connection.get(), MYSQL_INIT_COMMAND, "SET NAMES utf8");

    const auto connectionTimeoutSeconds = static_cast<unsigned>(connectTimeout().count());
    mysql_options(m_connection.get(), MYSQL_OPT_CONNECT_TIMEOUT, &connectionTimeoutSeconds);
}

void MySQLConnection::_openDatabase(const String& newConnectionString)
{
    const scoped_lock lock(m_mutex);
    _openDatabaseUnlocked(newConnectionString);
}

// Caller must hold m_mutex.
void MySQLConnection::_openDatabaseUnlocked(const String& newConnectionString)
{
    if (!activeUnlocked())
    {
        setInTransaction(false);
        if (!newConnectionString.empty())
        {
            connectionString(DatabaseConnectionString(newConnectionString));
        }

        initConnectionUnlocked();

        if (const auto& connString = connectionString();
            mysql_real_connect(m_connection.get(),
                               connString.hostName().c_str(),
                               connString.userName().c_str(),
                               connString.password().c_str(),
                               connString.databaseName().c_str(),
                               connString.portNumber(),
                               nullptr,
                               CLIENT_MULTI_RESULTS | CLIENT_LOCAL_FILES) == nullptr)
        {
            const String connectionError = mysql_error(m_connection.get());
            m_connection.reset();
            throw DatabaseException("Can't connect to MySQL: " + connectionError);
        }
    }
}

void MySQLConnection::closeDatabase()
{
    const scoped_lock lock(m_mutex);
    m_connection.reset();
}

DBHandle MySQLConnection::handle() const
{
    const scoped_lock lock(m_mutex);
    return handleUnlocked();
}

// Caller must hold m_mutex.
DBHandle MySQLConnection::handleUnlocked() const
{
    return bit_cast<DBHandle>(m_connection.get());
}

bool MySQLConnection::active() const
{
    const scoped_lock lock(m_mutex);
    return activeUnlocked();
}

// Caller must hold m_mutex.
bool MySQLConnection::activeUnlocked() const
{
    return m_connection != nullptr;
}

void MySQLConnection::executeCommand(const String& command)
{
    const scoped_lock lock(m_mutex);
    executeCommandUnlocked(command);
}

// Caller must hold m_mutex.
void MySQLConnection::executeCommandUnlocked(const String& command)
{
    if (m_connection == nullptr)
    {
        // Base open() would take the lock again - use the unlocked variant.
        _openDatabaseUnlocked("");
    }

    if (mysql_real_query(m_connection.get(), command.c_str(), static_cast<unsigned>(command.length())) != 0)
    {
        throwMySQLException(m_connection, "Can't execute " + command);
    }
}

void MySQLConnection::driverBeginTransaction()
{
    const scoped_lock lock(m_mutex);
    if (getInTransaction())
    {
        throwMySQLException(m_connection, "Transaction already started");
    }

    executeCommandUnlocked("BEGIN");
    setInTransaction(true);
}

void MySQLConnection::driverEndTransaction(bool commit)
{
    const scoped_lock lock(m_mutex);
    if (!getInTransaction())
    {
        throw DatabaseException("Transaction isn't started.");
    }

    const char* action = commit ? "COMMIT" : "ROLLBACK";
    executeCommandUnlocked(action);
    setInTransaction(false);
}

//-----------------------------------------------------------------------------------------------
String MySQLConnection::queryError(const Query* query) const
{
    const scoped_lock lock(m_mutex);
    return mysql_error(m_connection.get()) + string(", query: ") + query->sql();
}

void MySQLConnection::queryAllocStmt(Query* query)
{
    const scoped_lock lock(m_mutex);
    queryAllocStmtUnlocked(query);
}

// Caller must hold m_mutex.
void MySQLConnection::queryAllocStmtUnlocked(Query* query)
{
    queryFreeStmtUnlocked(query);
    const auto stmt = reinterpret_pointer_cast<uint8_t>(make_shared<MySQLStatement>(this, query->sql(), query->autoPrepare()));
    querySetStmt(query, stmt);
}

void MySQLConnection::queryFreeStmt(Query* query)
{
    const scoped_lock lock(m_mutex);
    queryFreeStmtUnlocked(query);
}

// Caller must hold m_mutex.
void MySQLConnection::queryFreeStmtUnlocked(Query* query)
{
    querySetStmt(query, nullptr);
    querySetPrepared(query, false);
}

void MySQLConnection::queryCloseStmt(Query* query)
{
    const scoped_lock lock(m_mutex);
    try
    {
        if (auto* statement = bit_cast<MySQLStatement*>(query->statement()))
        {
            statement->close();
        }
    }
    catch (const Exception& e)
    {
        THROW_QUERY_ERROR(query, e.what());
    }
}

void MySQLConnection::queryPrepare(Query* query)
{
    const scoped_lock lock(m_mutex);
    queryPrepareUnlocked(query);
}

// Caller must hold m_mutex.
void MySQLConnection::queryPrepareUnlocked(Query* query)
{
    if (query->prepared())
    {
        return;
    }

    auto* statement = bit_cast<MySQLStatement*>(query->statement());
    if (statement != nullptr)
    {
        try
        {
            statement->prepare(query->sql());
            statement->enumerateParams(query->params());
            querySetPrepared(query, true);
        }
        catch (const Exception& e)
        {
            THROW_QUERY_ERROR(query, e.what());
        }
    }
}

size_t MySQLConnection::queryColCount(Query* query)
{
    const scoped_lock lock(m_mutex);
    return queryColCountUnlocked(query);
}

// Caller must hold m_mutex.
size_t MySQLConnection::queryColCountUnlocked(Query* query)
{
    size_t      colCount;
    const auto* statement = bit_cast<MySQLStatement*>(query->statement());
    try
    {
        if (statement == nullptr)
        {
            throw DatabaseException("Query not opened");
        }
        colCount = statement->colCount();
    }
    catch (const Exception& e)
    {
        THROW_QUERY_ERROR(query, e.what());
    }
    return colCount;
}

void MySQLConnection::queryBindParameters(Query* query)
{
    const scoped_lock lock(m_mutex);
    queryBindParametersUnlocked(query);
}

// Caller must hold m_mutex.
void MySQLConnection::queryBindParametersUnlocked(Query* query)
{
    auto* statement = bit_cast<MySQLStatement*>(query->statement());
    try
    {
        if (statement == nullptr)
        {
            throw DatabaseException("Query not prepared");
        }
        statement->setParameterValues();
    }
    catch (const Exception& e)
    {
        THROW_QUERY_ERROR(query, e.what());
    }
}

void MySQLConnection::queryExecute(Query* query)
{
    const scoped_lock lock(m_mutex);
    queryExecuteUnlocked(query);
}

// Caller must hold m_mutex.
void MySQLConnection::queryExecuteUnlocked(Query* query)
{
    auto* statement = bit_cast<MySQLStatement*>(query->statement());
    try
    {
        if (statement == nullptr)
        {
            throw DatabaseException("Query is not prepared");
        }
        statement->execute(getInTransaction());
    }
    catch (const Exception& e)
    {
        THROW_QUERY_ERROR(query, e.what());
    }
}

void MySQLConnection::queryOpen(Query* query)
{
    // One lock for the whole open sequence: no other query can execute on this
    // connection between queryExecute() and the first queryFetch().
    const scoped_lock lock(m_mutex);
    queryOpenUnlocked(query);
}

// Caller must hold m_mutex.
void MySQLConnection::queryOpenUnlocked(Query* query)
{
    if (!activeUnlocked())
    {
        // Base open() would take the lock again - use the unlocked variant.
        _openDatabaseUnlocked("");
    }

    if (query->active())
    {
        return;
    }

    if (query->statement() == nullptr)
    {
        queryAllocStmtUnlocked(query);
    }

    if (query->autoPrepare())
    {
        if (!query->prepared())
        {
            queryPrepareUnlocked(query);
        }
        queryBindParametersUnlocked(query);
    }

    auto* statement = bit_cast<MySQLStatement*>(query->statement());

    queryExecuteUnlocked(query);
    if (const auto fieldCount = static_cast<short>(queryColCountUnlocked(query)); fieldCount < 1)
    {
        return;
    }

    querySetActive(query, true);
    if (query->fieldCount() == 0)
    {
        statement->bindResult(query->fields());
    }

    querySetEof(query, statement->eof());
    queryFetchUnlocked(query);
}

void MySQLConnection::queryFetch(Query* query)
{
    const scoped_lock lock(m_mutex);
    queryFetchUnlocked(query);
}

// Caller must hold m_mutex.
void MySQLConnection::queryFetchUnlocked(Query* query)
{
    if (!query->active())
    {
        THROW_QUERY_ERROR(query, "Dataset isn't open");
    }

    try
    {
        auto* statement = bit_cast<MySQLStatement*>(query->statement());

        statement->fetch();
        if (statement->eof())
        {
            querySetEof(query, true);
            return;
        }

        statement->readResultRow(query->fields());
    }
    catch (const Exception& e)
    {
        Query::throwError("MySQLConnection::queryFetch", e.what());
    }
}

void MySQLConnection::objectList(DatabaseObjectType objectType, Strings& objects)
{
    string objectsSQL;
    objects.clear();
    switch (objectType)
    {
        using enum DatabaseObjectType;
        case PROCEDURES:
            objectsSQL =
                "SELECT CONCAT(routine_schema, '.', routine_name) object_name "
                "FROM information_schema.routines "
                "WHERE routine_type = 'PROCEDURE'";
            break;
        case FUNCTIONS:
            objectsSQL =
                "SELECT CONCAT(routine_schema, '.', routine_name) object_name "
                "FROM information_schema.routines "
                "WHERE routine_type = 'FUNCTION'";
            break;
        case TABLES:
            objectsSQL =
                "SELECT CONCAT(table_schema, '.', table_name) object_name "
                "FROM information_schema.tables "
                "WHERE NOT table_schema IN ('mysql','information_schema')";
            break;
        case VIEWS:
            objectsSQL =
                "SELECT CONCAT(table_schema, '.', table_name) object_name "
                "FROM information_schema.views";
            break;
        case DATABASES:
            objectsSQL =
                "SHOW DATABASES where `Database` NOT IN ('information_schema','performance_schema','mysql')";
            break;
    }

    auto self = shared_from_this();
    if (self == nullptr)
    {
        throw DatabaseException("PoolDatabaseConnection is not created as shared_ptr");
    }

    Query query(self, objectsSQL);
    try
    {
        query.open();
        while (!query.eof())
        {
            objects.push_back(query[static_cast<uint32_t>(0)].asString());
            query.next();
        }
        query.close();
    }
    catch (const Exception& e)
    {
        throw DatabaseException(format("Error fetching system info: {}", e.what()));
    }
}

void MySQLConnection::executeBatchSQL(const Strings& batchSQL, Strings* errors)
{
    auto matchStatementEnd = make_shared<RegularExpression>("(;\\s*)$");

    static const RegularExpression matchDelimiterChange("^DELIMITER\\s+(\\S+)");
    static const RegularExpression matchEscapeChars("([$.])", "g");
    static const RegularExpression matchCommentRow("^\\s*--");

    Strings statements;
    String  statement;
    for (auto row: batchSQL)
    {
        row = row.trim();
        if (row.empty() || matchCommentRow.matches(row))
        {
            continue;
        }

        if (auto matches = matchDelimiterChange.m(row); matches)
        {
            auto delimiter = matches[0].value;
            delimiter = matchEscapeChars.s(delimiter, R"(\\1)");
            matchStatementEnd = make_shared<RegularExpression>("(" + delimiter + ")(\\s*|-- .*)$");
            statement = "";
            continue;
        }

        if (matchStatementEnd->matches(row))
        {
            row = matchStatementEnd->s(row, "");
            statement += row;
            statements.push_back(statement);
            statement = "";
            continue;
        }

        statement += row + "\n";
    }

    if (!trim(statement).empty())
    {
        statements.push_back(statement);
    }

    auto self = shared_from_this();
    if (self == nullptr)
    {
        throw DatabaseException("PoolDatabaseConnection is not created as shared_ptr");
    }

    for (const auto& stmt: statements)
    {
        Query query(self, stmt, false);
        try
        {
            query.exec();
        }
        catch (const Exception& e)
        {
            stringstream error;
            error << e.what() << ", query: " << query.sql();
            if (errors != nullptr)
            {
                errors->push_back(error.str());
            }
            else
            {
                throw DatabaseException(error.str());
            }
        }
    }
}

namespace {
/// In-memory source for LOAD DATA LOCAL INFILE: the client library asks for chunks through these
/// callbacks, so no temporary file is needed.
struct LoadDataBuffer
{
    const char* data;
    size_t      size;
    size_t      offset;
};

int loadDataInit(void** ptr, const char*, void* userdata)
{
    auto* buffer = static_cast<LoadDataBuffer*>(userdata);
    buffer->offset = 0;
    *ptr = buffer;
    return 0;
}

int loadDataRead(void* ptr, char* buffer, const unsigned int length)
{
    auto* source = static_cast<LoadDataBuffer*>(ptr);
    if (source->offset >= source->size)
    {
        return 0;
    }
    const auto count = std::min<size_t>(length, source->size - source->offset);
    memcpy(buffer, source->data + source->offset, count);
    source->offset += count;
    return static_cast<int>(count);
}

void loadDataEnd(void*)
{
}

int loadDataError(void*, char* buffer, const unsigned int length)
{
    if (length > 0)
    {
        snprintf(buffer, length, "Can't read bulk data");
    }
    return 1;
}

void appendLoadDataField(String& line, const Variant& value)
{
    if (value.isNull())
    {
        line += "\\N";
        return;
    }

    using enum VariantDataType;
    switch (value.dataType())
    {
        case VAR_INT:
            line += std::to_string(value.asInteger());
            break;

        case VAR_INT64:
            line += std::to_string(value.asInt64());
            break;

        case VAR_FLOAT:
            line += value.asString();
            break;

        case VAR_BOOL:
            line += value.asBool() ? "1" : "0";
            break;

        case VAR_DATE:
            line += value.asDate().dateString();
            break;

        case VAR_DATE_TIME:
            line += value.asDateTime().isoDateTimeString();
            break;

        default: {
            // The same escapes work for text and for binary data, including NUL bytes.
            for (const char c: value.asString())
            {
                switch (c)
                {
                    case '\\': line += "\\\\"; break;
                    case '\t': line += "\\t"; break;
                    case '\n': line += "\\n"; break;
                    case '\r': line += "\\r"; break;
                    case '\0': line += "\\0"; break;
                    case '\032': line += "\\Z"; break;
                    default:   line += c; break;
                }
            }
        }
    }
}

bool localInfileEnabled(MYSQL* connection)
{
    if (mysql_real_query(connection, "SELECT @@local_infile", 21) != 0)
    {
        return false;
    }
    MYSQL_RES* result = mysql_store_result(connection);
    bool       enabled = false;
    if (result != nullptr)
    {
        if (MYSQL_ROW row = mysql_fetch_row(result); row != nullptr && row[0] != nullptr)
        {
            enabled = strcmp(row[0], "1") == 0 || strcmp(row[0], "ON") == 0;
        }
        mysql_free_result(result);
    }
    return enabled;
}

void executeLoadData(MYSQL* connection, const String& sql, const String& payload)
{
    LoadDataBuffer buffer {payload.data(), payload.size(), 0};
    mysql_set_local_infile_handler(connection, loadDataInit, loadDataRead, loadDataEnd, loadDataError, &buffer);
    const int rc = mysql_real_query(connection, sql.c_str(), static_cast<unsigned long>(sql.size()));
    mysql_set_local_infile_default(connection);
    if (rc != 0)
    {
        throw DatabaseException(String("LOAD DATA failed: ") + mysql_error(connection));
    }
}

void appendSqlLiteral(String& sql, const Variant& value)
{
    if (value.isNull())
    {
        sql += "NULL";
        return;
    }

    using enum VariantDataType;
    switch (value.dataType())
    {
        case VAR_INT:
            sql += std::to_string(value.asInteger());
            break;

        case VAR_INT64:
            sql += std::to_string(value.asInt64());
            break;

        case VAR_FLOAT:
            sql += value.asString();
            break;

        case VAR_BOOL:
            sql += value.asBool() ? "1" : "0";
            break;

        case VAR_BUFFER: {
            const auto& bytes = value.get<Buffer>();
            sql += "X'";
            constexpr char hex[] = "0123456789ABCDEF";
            for (size_t i = 0; i < bytes.bytes(); ++i)
            {
                sql += hex[bytes.data()[i] >> 4];
                sql += hex[bytes.data()[i] & 0x0f];
            }
            sql += '\'';
            break;
        }

        default: {
            sql += '\'';
            for (const char c: value.asString())
            {
                switch (c)
                {
                    case '\\': sql += "\\\\"; break;
                    case '\'': sql += "''"; break;
                    case '\n': sql += "\\n"; break;
                    case '\r': sql += "\\r"; break;
                    default:   sql += c; break;
                }
            }
            sql += '\'';
        }
    }
}

String bulkKeyColumnType(const VariantVector& keys)
{
    for (const auto& key: keys)
    {
        if (key.isNull())
        {
            continue;
        }
        using enum VariantDataType;
        switch (key.dataType())
        {
            case VAR_INT:
            case VAR_INT64:
            case VAR_BOOL:
                return "BIGINT";
            case VAR_FLOAT:
                return "DOUBLE";
            case VAR_DATE:
            case VAR_DATE_TIME:
                return "DATETIME";
            default:
                return "VARCHAR(1024)";
        }
    }
    return "VARCHAR(1024)";
}
} // namespace

int64_t MySQLConnection::lastInsertId() const
{
    const scoped_lock lock(m_mutex);
    if (m_connection == nullptr)
    {
        throw Exception("Database connection is not open");
    }
    // Connection-local in MySQL: no round trip and no race with another connection's inserts.
    return static_cast<int64_t>(mysql_insert_id(m_connection.get()));
}

void MySQLConnection::bulkInsert(const String& tableName, const String& autoIncrementColumnName,
                                 const Strings& columnNames, vector<VariantVector>& data, size_t groupSize,
                                 vector<int64_t>* insertedIds)
{
    if (groupSize == 0)
    {
        throw Exception("Group size must be greater than zero");
    }
    if (tableName.empty())
    {
        throw Exception("Table name is empty");
    }
    if (columnNames.empty())
    {
        throw Exception("Column list is empty");
    }
    if (data.empty())
    {
        return;
    }

    // LOAD DATA cannot report generated keys. Keep the correct generic path for that case.
    if ((insertedIds != nullptr && !autoIncrementColumnName.empty()) ||
        (!autoIncrementColumnName.empty() && columnNames.indexOf(autoIncrementColumnName) >= 0))
    {
        PoolDatabaseConnection::bulkInsert(tableName, autoIncrementColumnName, columnNames, data, groupSize, insertedIds);
        return;
    }

    String payload;
    payload.reserve(data.size() * columnNames.size() * 16);
    for (const auto& row: data)
    {
        if (row.size() != columnNames.size())
        {
            throw DatabaseException("Row size mismatch in bulk insert");
        }
        for (size_t column = 0; column < row.size(); ++column)
        {
            if (column > 0)
            {
                payload += '\t';
            }
            appendLoadDataField(payload, row[column]);
        }
        payload += '\n';
    }

    const String sql = "LOAD DATA LOCAL INFILE 'sptk_bulk' INTO TABLE " + tableName +
                       " FIELDS TERMINATED BY '\\t' LINES TERMINATED BY '\\n' (" + columnNames.join(",") + ")";

    if (!active())
    {
        open();
    }

    bool localInfile = false;
    {
        // Transaction helpers take the same mutex, so they must not run while it is held.
        const scoped_lock lock(m_mutex);
        localInfile = localInfileEnabled(m_connection.get());
    }
    if (!localInfile)
    {
        // LOCAL INFILE is often disabled by policy. Build literal multi-row INSERTs instead: one
        // mysql_real_query per chunk, no prepared statement and no per-parameter binding.
        const bool wasInTransaction = getInTransaction();
        if (!wasInTransaction)
        {
            beginTransaction();
        }
        const size_t chunkSize = max<size_t>(groupSize, 1000);
        try
        {
            for (size_t offset = 0; offset < data.size(); offset += chunkSize)
            {
                String sql = "INSERT INTO " + tableName + " (" + columnNames.join(",") + ") VALUES ";
                const auto end = min(offset + chunkSize, data.size());
                for (size_t row = offset; row < end; ++row)
                {
                    if (row > offset)
                    {
                        sql += ',';
                    }
                    sql += '(';
                    for (size_t column = 0; column < data[row].size(); ++column)
                    {
                        if (column > 0)
                        {
                            sql += ',';
                        }
                        appendSqlLiteral(sql, data[row][column]);
                    }
                    sql += ')';
                }
                const scoped_lock lock(m_mutex);
                if (mysql_real_query(m_connection.get(), sql.c_str(), static_cast<unsigned long>(sql.size())) != 0)
                {
                    throw DatabaseException(String("Bulk insert failed: ") + mysql_error(m_connection.get()));
                }
            }
            if (!wasInTransaction)
            {
                commitTransaction();
            }
        }
        catch (...)
        {
            if (!wasInTransaction)
            {
                try
                {
                    rollbackTransaction();
                }
                catch (const Exception&)
                {
                    // Preserve the original error.
                }
            }
            throw;
        }
        return;
    }

    const bool wasInTransaction = getInTransaction();
    if (!wasInTransaction)
    {
        beginTransaction();
    }

    auto committed = false;
    try
    {
        {
            const scoped_lock lock(m_mutex);
            executeLoadData(m_connection.get(), sql, payload);
        }
        if (insertedIds != nullptr)
        {
            insertedIds->clear();
        }
        if (!wasInTransaction)
        {
            commitTransaction();
        }
        committed = true;
    }
    catch (...)
    {
        {
            const scoped_lock lock(m_mutex);
            mysql_set_local_infile_default(m_connection.get());
        }
        if (!wasInTransaction && !committed)
        {
            try
            {
                rollbackTransaction();
            }
            catch (const Exception&)
            {
                // Preserve the original error.
            }
        }
        throw;
    }
}

void MySQLConnection::bulkDelete(const String& tableName, const String& keyColumnName, const VariantVector& keys)
{
    if (tableName.empty() || keyColumnName.empty())
    {
        throw Exception("Table name and key column are required");
    }
    if (keys.empty())
    {
        return;
    }

    String payload;
    payload.reserve(keys.size() * 24);
    for (const auto& key: keys)
    {
        appendLoadDataField(payload, key);
        payload += '\n';
    }

    if (!active())
    {
        open();
    }

    bool localInfile = false;
    {
        const scoped_lock lock(m_mutex);
        localInfile = localInfileEnabled(m_connection.get());
    }
    if (!localInfile)
    {
        // A single IN list with literal keys: one query per 1000 keys instead of one per 50.
        const bool wasInTransaction = getInTransaction();
        if (!wasInTransaction)
        {
            beginTransaction();
        }
        constexpr size_t chunkSize = 1000;
        try
        {
            for (size_t offset = 0; offset < keys.size(); offset += chunkSize)
            {
                String sql = "DELETE FROM " + tableName + " WHERE " + keyColumnName + " IN (";
                const auto end = min(offset + chunkSize, keys.size());
                for (size_t i = offset; i < end; ++i)
                {
                    if (i > offset)
                    {
                        sql += ',';
                    }
                    appendSqlLiteral(sql, keys[i]);
                }
                sql += ')';
                const scoped_lock lock(m_mutex);
                if (mysql_real_query(m_connection.get(), sql.c_str(), static_cast<unsigned long>(sql.size())) != 0)
                {
                    throw DatabaseException(String("Bulk delete failed: ") + mysql_error(m_connection.get()));
                }
            }
            if (!wasInTransaction)
            {
                commitTransaction();
            }
        }
        catch (...)
        {
            if (!wasInTransaction)
            {
                try
                {
                    rollbackTransaction();
                }
                catch (const Exception&)
                {
                    // Preserve the original error.
                }
            }
            throw;
        }
        return;
    }

    const bool wasInTransaction = getInTransaction();
    if (!wasInTransaction)
    {
        beginTransaction();
    }

    auto committed = false;
    try
    {
        {
            const scoped_lock lock(m_mutex);
            const String createSql = "CREATE TEMPORARY TABLE sptk_bulk_keys (k " + bulkKeyColumnType(keys) +
                                     " NOT NULL, PRIMARY KEY(k))";
            if (mysql_real_query(m_connection.get(), createSql.c_str(),
                                 static_cast<unsigned long>(createSql.size())) != 0)
            {
                throw DatabaseException(String("Can't create temporary key table: ") + mysql_error(m_connection.get()));
            }

            const String loadSql = "LOAD DATA LOCAL INFILE 'sptk_bulk_keys' INTO TABLE sptk_bulk_keys "
                                   "FIELDS TERMINATED BY '\\t' LINES TERMINATED BY '\\n' (k)";
            executeLoadData(m_connection.get(), loadSql, payload);

            const String deleteSql = "DELETE t FROM " + tableName + " t INNER JOIN sptk_bulk_keys k ON t." +
                                     keyColumnName + " = k.k";
            if (mysql_real_query(m_connection.get(), deleteSql.c_str(),
                                 static_cast<unsigned long>(deleteSql.size())) != 0)
            {
                throw DatabaseException(String("Bulk delete failed: ") + mysql_error(m_connection.get()));
            }

            constexpr char dropSql[] = "DROP TEMPORARY TABLE sptk_bulk_keys";
            mysql_real_query(m_connection.get(), dropSql, sizeof(dropSql) - 1);
        }

        if (!wasInTransaction)
        {
            commitTransaction();
        }
        committed = true;
    }
    catch (...)
    {
        {
            const scoped_lock lock(m_mutex);
            mysql_set_local_infile_default(m_connection.get());
            mysql_real_query(m_connection.get(), "DROP TEMPORARY TABLE IF EXISTS sptk_bulk_keys", 45);
        }
        if (!wasInTransaction && !committed)
        {
            try
            {
                rollbackTransaction();
            }
            catch (const Exception&)
            {
                // Preserve the original error.
            }
        }
        throw;
    }
}

String MySQLConnection::driverDescription() const
{
    const scoped_lock lock(m_mutex);
    if (m_connection != nullptr)
    {
        return string("MySQL ") + mysql_get_server_info(m_connection.get());
    }
    return "MySQL";
}

string MySQLConnection::paramMark(unsigned)
{
    return "?";
}

void MySQLConnection::queryColAttributes(Query*, int16_t, int16_t, int32_t&)
{
    notImplemented("queryColAttributes");
}

void MySQLConnection::queryColAttributes(Query*, int16_t, int16_t, char*, int)
{
    notImplemented("queryColAttributes");
}

SynchronizedMap<MySQLConnection*, shared_ptr<MySQLConnection>> MySQLConnection::s_mysqlConnections;

[[maybe_unused]] void* mysqlCreateConnection(const char* connectionString, size_t connectionTimeoutSeconds)
{
    const auto connection = make_shared<MySQLConnection>(connectionString, chrono::seconds(connectionTimeoutSeconds));
    MySQLConnection::s_mysqlConnections.insert(connection.get(), connection);
    return connection.get();
}

[[maybe_unused]] void mysqlDestroyConnection(void* connection)
{
    MySQLConnection::s_mysqlConnections.erase(bit_cast<MySQLConnection*>(connection));
}
