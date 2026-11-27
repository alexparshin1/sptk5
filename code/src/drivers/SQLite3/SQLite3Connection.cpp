/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
╟──────────────────────────────────────────────────────────────────────────────╢
║  copyright            © 1999-2026 Alexey Parshin                             ║
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

#include <sptk5/sptk.h>

#ifdef HAVE_SQLITE3

#include "sptk5/db/BulkQuery.h"
#include <sptk5/cutils>
#include <sptk5/db/DatabaseField.h>
#include <sptk5/db/SQLite3Connection.h>
#include <sstream> // libc++

namespace sptk {
class SQLite3Field
    : public DatabaseField
{
    friend class SQLite3Connection;

public:
    explicit SQLite3Field(const std::string& fieldName)
        : DatabaseField(fieldName, 0, VariantDataType::VAR_BUFFER, 0, 0)
    {
    }

    using DatabaseField::operator=;
};

} // namespace sptk

using namespace std;
using namespace sptk;

SQLite3Connection::SQLite3Connection(const String& connectionString, const chrono::seconds connectTimeout)
    : PoolDatabaseConnection(connectionString, DatabaseConnectionType::SQLITE3, connectTimeout)
{
}

String SQLite3Connection::nativeConnectionString() const
{
    if (connectionString().portNumber() > 0)
    {
        throw DatabaseException("Invalid connection string");
    }

    const auto dbFileName = static_cast<string>(connectionString().databaseName());
    const auto schema = connectionString().schema();
    return schema.empty() ? dbFileName : dbFileName + "/" + schema;
}

chrono::minutes SQLite3Connection::getSessionTimezoneOffset()
{
    // Read the server timestamp directly: going through Query would require a shared_ptr to this
    // connection, so a stack-allocated SQLite3Connection could not be opened at all.
    const String sql("SELECT CURRENT_TIMESTAMP");
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_connect.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        throw DatabaseException(String("Can't prepare timezone query: ") + sqlite3_errmsg(m_connect.get()));
    }

    const auto localBefore = DateTime::Now();
    const int  stepResult = sqlite3_step(stmt);
    const auto localAfter = DateTime::Now();

    String serverTime;
    if (stepResult == SQLITE_ROW)
    {
        if (const auto* text = sqlite3_column_text(stmt, 0); text != nullptr)
        {
            serverTime = reinterpret_cast<const char*>(text);
        }
    }
    sqlite3_finalize(stmt);

    if (stepResult != SQLITE_ROW || serverTime.empty())
    {
        throw DatabaseException("Can't read server timestamp for timezone offset");
    }

    const DateTime serverDateTime(serverTime.c_str());
    // Sample local time on both sides of the query to remove most of the round-trip bias.
    const DateTime midpoint = localBefore + (localAfter - localBefore) / 2;
    return chrono::duration_cast<chrono::minutes>(serverDateTime - midpoint);
}

void SQLite3Connection::_openDatabase(const String& newConnectionString)
{
    const scoped_lock lock(m_mutex);
    if (!active())
    {
        clearPreparedStatements();
        setInTransaction(false);

        if (!newConnectionString.empty())
        {
            connectionString(DatabaseConnectionString(newConnectionString));
        }

        sqlite3* connect = nullptr;
        if (const auto dbFileName = this->nativeConnectionString();
            sqlite3_open(dbFileName.c_str(), &connect) != SQLITE_OK)
        {
            if (connect != nullptr)
            {
                const String error = sqlite3_errmsg(connect);
                sqlite3_close(connect);
                m_connect.reset();
                throw DatabaseException(error + " '" + dbFileName + "'");
            }
            throw DatabaseException("Can't open the database '" + dbFileName + "'");
        }

        // The deleter owns the handle it was given and must not call back into this object: a
        // closeDatabase() in progress may be holding the mutex or replacing m_connect. The shared
        // flag lets an explicit close disable it after sqlite3_close() has already succeeded.
        m_closeState = make_shared<bool>(false);
        m_connect = shared_ptr<sqlite3>(connect,
                                        [closeState = m_closeState](sqlite3* ptr)
                                        {
                                            if (!*closeState && ptr != nullptr)
                                            {
                                                sqlite3_close(ptr);
                                            }
                                        });

        // Wait for a database another connection is writing, rather than failing the moment it is
        // found busy - which is what SQLite does when no timeout is set, and no timeout was.
        //
        // A pool hands out several connections to one file, so a read arriving while a write is
        // committing is ordinary rather than exceptional. Without this the read returns
        // SQLITE_BUSY immediately and the caller sees an error that says nothing about what is
        // wrong; with it, it waits the few milliseconds the writer needs. Ten seconds is far
        // longer than any write here takes, and is a limit rather than a delay: a database that
        // is genuinely stuck still reports it rather than hanging for good.
        constexpr int busyTimeoutMs = 10000;
        sqlite3_busy_timeout(m_connect.get(), busyTimeoutMs);

        const Strings pragmas{
            // SQLite parses a REFERENCES clause whether or not this is on, and silently enforces
            // nothing when it is off - which is its default. A schema whose foreign keys are
            // believed to hold and are not is worse off than one that declares none: the cascade
            // never runs, the orphan is never refused, and nothing anywhere says so.
            "pragma foreign_keys = ON",
            "pragma journal_mode = WAL",
            // FULL, not off. Off does not wait for a commit to reach the disk, so a power cut or a
            // kernel crash loses transactions that were reported as committed - a reasonable trade
            // for a queue that can replay, and the wrong one for the accounts and rules a broker is
            // about to keep here, where a password change that quietly did not happen is worse than
            // a slow one. NORMAL would be the middle ground in WAL mode; the choice wants to be a
            // connection parameter rather than a constant, and is not one yet.
            "pragma synchronous = full",
            "pragma temp_store = memory",
            "pragma mmap_size = 30000000000",
            "pragma auto_vacuum = incremental",
        };
        for (const auto& pragma: pragmas)
        {
            if (char* zErrMsg = nullptr;
                sqlite3_exec(m_connect.get(), pragma.c_str(), nullptr, nullptr, &zErrMsg) != SQLITE_OK)
            {
                const String error(zErrMsg);
                sqlite3_free(zErrMsg);
                throw DatabaseException(error);
            }
        }

        m_sessionTimezoneOffset = getSessionTimezoneOffset();
    }
}

void SQLite3Connection::closeDatabase()
{
    const scoped_lock lock(m_mutex);
    disconnectAllQueries();
    clearPreparedStatements();
    if (m_connect == nullptr)
    {
        return;
    }

    switch (sqlite3_close(m_connect.get()))
    {
        case SQLITE_OK:
            break;
        case SQLITE_BUSY:
            throw DatabaseException("Failed to close SQLite3 connection: The database is busy.");
        default:
            throw DatabaseException("Failed to close SQLite3 connection");
    }
    *m_closeState = true;
    m_connect = nullptr;
}

DBHandle SQLite3Connection::handle() const
{
    const scoped_lock lock(m_mutex);
    return bit_cast<DBHandle>(m_connect.get());
}

bool SQLite3Connection::active() const
{
    const scoped_lock lock(m_mutex);
    return m_connect != nullptr;
}

void SQLite3Connection::driverBeginTransaction()
{
    const scoped_lock lock(m_mutex);
    if (m_connect == nullptr)
    {
        open();
    }

    if (getInTransaction())
    {
        throw DatabaseException("Transaction already started.");
    }


    if (char* zErrMsg = nullptr;
        sqlite3_exec(m_connect.get(), "BEGIN TRANSACTION", nullptr, nullptr, &zErrMsg) != SQLITE_OK)
    {
        const String error(zErrMsg);
        sqlite3_free(zErrMsg);
        throw DatabaseException(error);
    }

    setInTransaction(true);
}

void SQLite3Connection::driverEndTransaction(const bool commit)
{
    const scoped_lock lock(m_mutex);
    if (!getInTransaction())
    {
        throw DatabaseException("Transaction isn't started.");
    }

    const char* action = commit ? "COMMIT" : "ROLLBACK";

    if (char* zErrMsg = nullptr;
        sqlite3_exec(m_connect.get(), action, nullptr, nullptr, &zErrMsg) != SQLITE_OK)
    {
        const String error(zErrMsg);
        sqlite3_free(zErrMsg);
        throw DatabaseException(error);
    }

    setInTransaction(false);
}

//-----------------------------------------------------------------------------------------------

String SQLite3Connection::queryError(const Query*) const
{
    const scoped_lock lock(m_mutex);
    return m_connect == nullptr ? "Database connection is not open" : sqlite3_errmsg(m_connect.get());
}

// Doesn't allocate a statement, but makes sure the previously allocated stmt is released.
void SQLite3Connection::queryAllocStmt(Query* query)
{
    queryFreeStmt(query);

    const scoped_lock lock(m_mutex);
    if (query->autoPrepare())
    {
        if (const auto cached = m_preparedStatements.find(query->sql()); cached != m_preparedStatements.end())
        {
            // Handed out exclusively: a statement and its result row cannot be shared by two queries.
            const SStmtHandle handle = cached->second;
            m_preparedStatements.erase(cached);
            m_preparedStatementKeys.erase(find(m_preparedStatementKeys.begin(), m_preparedStatementKeys.end(),
                                                query->sql()));
            sqlite3_reset(bit_cast<SQLHSTMT>(handle.get()));
            querySetStmt(query, handle);
            querySetPrepared(query, true);
            return;
        }
    }
    // No new statement here: queryPrepare() compiles it, or queryExecDirect uses a temporary one.
}

void SQLite3Connection::queryFreeStmt(Query* query)
{
    std::vector<SStmtHandle> evicted;
    SStmtHandle              discarded;
    {
        const scoped_lock lock(m_mutex);
        const SStmtHandle  handle = queryStatement(query);
        if (query->prepared())
        {
            evicted = cacheStatement(query->sql(), handle);
        }
        else
        {
            // Dropping the shared_ptr finalizes the statement; do that after unlocking.
            discarded = handle;
        }
        querySetStmt(query, nullptr);
        querySetPrepared(query, false);
    }
}

std::vector<SStmtHandle> SQLite3Connection::cacheStatement(const String& sql, const SStmtHandle& statement)
{
    std::vector<SStmtHandle> evicted;
    if (sql.empty() || statement == nullptr)
    {
        return evicted;
    }

    sqlite3_reset(bit_cast<SQLHSTMT>(statement.get()));
    sqlite3_clear_bindings(bit_cast<SQLHSTMT>(statement.get()));

    if (const auto existing = m_preparedStatements.find(sql); existing != m_preparedStatements.end())
    {
        evicted.push_back(existing->second);
        m_preparedStatements.erase(existing);
        m_preparedStatementKeys.erase(find(m_preparedStatementKeys.begin(), m_preparedStatementKeys.end(), sql));
    }

    m_preparedStatements.emplace(sql, statement);
    m_preparedStatementKeys.push_back(sql);

    while (m_preparedStatementKeys.size() > maxCachedStatements)
    {
        const String oldest = m_preparedStatementKeys.front();
        m_preparedStatementKeys.pop_front();
        if (const auto itor = m_preparedStatements.find(oldest); itor != m_preparedStatements.end())
        {
            evicted.push_back(itor->second);
            m_preparedStatements.erase(itor);
        }
    }
    return evicted;
}

void SQLite3Connection::clearPreparedStatements()
{
    const scoped_lock lock(m_mutex);
    m_preparedStatements.clear();
    m_preparedStatementKeys.clear();
}

void SQLite3Connection::queryCloseStmt(Query* query)
{
    const scoped_lock lock(m_mutex);

    if (auto* stmt = bit_cast<SQLHSTMT>(query->statement());
        stmt != nullptr)
    {
        sqlite3_reset(stmt);
    }
}

void SQLite3Connection::queryPrepare(Query* query)
{
    const scoped_lock lock(m_mutex);

    SQLHSTMT hStmt = nullptr;

    if (const char* pzTail = nullptr;
        sqlite3_prepare_v2(m_connect.get(), query->sql().c_str(), static_cast<int>(query->sql().length()), &hStmt, &pzTail) !=
        SQLITE_OK)
    {
        const char* errorMsg = sqlite3_errmsg(m_connect.get());
        throw DatabaseException(errorMsg, source_location::current(), query->sql());
    }

    const auto statement = shared_ptr<uint8_t>(bit_cast<StmtHandle>(hStmt),
                                               [](const StmtHandle ptr)
                                               {
                                                   auto* stmt = bit_cast<SQLHSTMT>(ptr);
                                                   sqlite3_finalize(stmt);
                                               });
    querySetStmt(query, statement);
    if (!statement)
    {
        throw DatabaseException("Can't prepare SQL statement");
    }
    querySetPrepared(query, true);
}

void SQLite3Connection::queryExecute(Query* query)
{
    const scoped_lock lock(m_mutex);

    if (!query->prepared())
    {
        throw DatabaseException("Query isn't prepared");
    }
}

size_t SQLite3Connection::queryColCount(Query* query)
{
    const scoped_lock lock(m_mutex);

    auto* stmt = bit_cast<SQLHSTMT>(query->statement());
    if (stmt == nullptr)
    {
        throw DatabaseException("Statement isn't open", source_location::current(), query->sql());
    }
    return static_cast<size_t>(sqlite3_column_count(stmt));
}

void SQLite3Connection::queryBindParameters(Query* query)
{
    const scoped_lock lock(m_mutex);

    auto* stmt = bit_cast<SQLHSTMT>(query->statement());
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);

    for (uint32_t i = 0; i < query->paramCount(); ++i)
    {
        bindParameter(query, i);
    }
}

void SQLite3Connection::bindParameter(Query* query, const uint32_t paramNumber)
{
    auto*                 stmt = bit_cast<SQLHSTMT>(query->statement());
    QueryParameter*       parameter = &query->param(paramNumber);
    const VariantDataType parameterType = parameter->dataType();

    for (unsigned j = 0; j < parameter->bindCount(); ++j)
    {
        int        res;
        const auto parameterBindNumber = static_cast<short>(parameter->bindIndex(j) + 1);

        if (parameter->isNull())
        {
            res = sqlite3_bind_null(stmt, parameterBindNumber);
        }
        else
        {
            switch (parameterType)
            {
                    using enum VariantDataType;
                case VAR_BOOL:
                    res = sqlite3_bind_int(stmt, parameterBindNumber, parameter->get<bool>());
                    break;

                case VAR_INT:
                    res = sqlite3_bind_int(stmt, parameterBindNumber, parameter->get<int>());
                    break;

                case VAR_INT64:
                    res = sqlite3_bind_int64(stmt, parameterBindNumber, parameter->get<int64_t>());
                    break;

                case VAR_FLOAT:
                    res = sqlite3_bind_double(stmt, parameterBindNumber, parameter->get<double>());
                    break;

                case VAR_DATE:
                case VAR_DATE_TIME:
                    res = transformDateTimeParameter(stmt, parameter, parameterBindNumber, parameterType);
                    break;

                case VAR_STRING:
                case VAR_TEXT:
                    res = sqlite3_bind_text(stmt, parameterBindNumber, parameter->getString(), static_cast<int>(parameter->dataSize()),
                                            SQLITE_TRANSIENT);
                    break;

                case VAR_BUFFER:
                    res = sqlite3_bind_blob(stmt, parameterBindNumber, parameter->getString(), static_cast<int>(parameter->dataSize()),
                                            SQLITE_TRANSIENT);
                    break;

                default:
                    throw DatabaseException(
                        format("Unsupported parameter type ({}) for parameter '{}'", static_cast<int>(parameter->dataType()), parameter->name()));
            }
        }

        if (res != SQLITE_OK)
        {
            const String error = sqlite3_errmsg(m_connect.get());
            querySetStmt(query, nullptr);
            throw DatabaseException(
                error + ", in binding parameter '" + parameter->name() + "'",
                source_location::current(), query->sql());
        }
    }
}

int SQLite3Connection::transformDateTimeParameter(sqlite3_stmt* stmt, QueryParameter* param, const short paramBindNumber, const VariantDataType dataType)
{
    const auto dt = dataType == VariantDataType::VAR_DATE ? param->get<DateTime>().date() : param->get<DateTime>();
    param->setString(dt.isoDateTimeString());
    const auto rc = sqlite3_bind_text(stmt, paramBindNumber, param->getString(), static_cast<int>(param->dataSize()),
                                      SQLITE_TRANSIENT);
    return rc;
}

void SQLite3Connection::queryOpen(Query* query)
{
    if (!active())
    {
        open();
    }

    if (query->active())
    {
        return;
    }

    if (active() && query->statement() == nullptr)
    {
        queryAllocStmt(query);
    }

    if (!query->prepared())
    {
        queryPrepare(query);
    }

    queryBindParameters(query);
    queryExecute(query);

    const auto count = static_cast<short>(queryColCount(query));

    query->fields().clear();

    auto* stmt = bit_cast<SQLHSTMT>(query->statement());

    if (count < 1)
    {
        if (sqlite3_step(stmt) != SQLITE_DONE)
        {
            const String error = queryError(query);
            queryCloseStmt(query);
            throw DatabaseException(error, source_location::current(), query->sql());
        }
        return;
    }

    querySetActive(query, true);

    // Reading the column attributes
    for (short column = 1; column <= count; ++column)
    {
        String columnName(sqlite3_column_name(stmt, column - 1));
        if (columnName.empty())
        {
            columnName = format("column_{}", column);
        }

        auto field = make_shared<SQLite3Field>(columnName);
        query->fields().push_back(field);
    }

    querySetEof(query, false);

    queryFetch(query);
}

namespace {
uint32_t trimField(char* str, const uint32_t length)
{
    if (length == 0)
    {
        return 0;
    }

    char*      p = str + length - 1;
    const char ch = str[0];
    str[0] = '!';

    while (*p == ' ')
    {
        --p;
    }

    *(++p) = 0;

    if (ch == ' ' && str[1] == 0)
    {
        return 0;
    }

    str[0] = ch;
    return static_cast<uint32_t>(p - str);
}
} // namespace

void SQLite3Connection::queryFetch(Query* query)
{
    static const RegularExpression matchDateTime(R"(^\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2}$)");

    if (!query->active())
    {
        throw DatabaseException("Dataset isn't open", source_location::current(), query->sql());
    }

    auto* statement = bit_cast<SQLHSTMT>(query->statement());
    if (statement == nullptr)
    {
        throw DatabaseException("Statement isn't open", source_location::current(), query->sql());
    }

    // Only the SQLite calls need the connection lock; converting values into fields does not.
    const auto locked = [&](auto&& fn) {
        const scoped_lock lock(m_mutex);
        return fn();
    };

    switch (locked([&] { return sqlite3_step(statement); }))
    {
        case SQLITE_DONE:
            querySetEof(query, true);
            return;

        case SQLITE_ROW:
            break;

        default:
            throw DatabaseException(queryError(query), source_location::current(), query->sql());
    }

    const auto fieldCount = query->fieldCount();
    if (fieldCount == 0)
    {
        return;
    }

    chrono::minutes sessionOffset;
    {
        const scoped_lock lock(m_mutex);
        sessionOffset = m_sessionTimezoneOffset;
    }

    SQLite3Field* field = nullptr;
    for (uint32_t column = 0; column < fieldCount; ++column)
    {
        try
        {
            field = bit_cast<SQLite3Field*>(&(*query)[column]);
            const auto columnIndex = static_cast<int>(column);

            auto fieldType = static_cast<short>(field->fieldType());
            if (fieldType == 0 || fieldType == SQLITE_NULL)
            {
                fieldType = static_cast<short>(locked([&] { return sqlite3_column_type(statement, columnIndex); }));
                field->setFieldType(fieldType, 0, 0);
            }

            if (fieldType == SQLITE_NULL)
            {
                setFieldToNull(field, fieldType);
                continue;
            }

            auto dataLength = static_cast<uint32_t>(locked([&] { return sqlite3_column_bytes(statement, columnIndex); }));

            // Zero-length text/blob is a value, not NULL. sqlite3_column_type() above is the only
            // reliable NULL test.
            if (dataLength == 0 && (fieldType == SQLITE_TEXT || fieldType == SQLITE_BLOB))
            {
                field->setBuffer(nullptr, 0, VariantDataType::VAR_BUFFER);
                field->dataSize(0);
                continue;
            }

            if (dataLength != 0)
            {
                switch (fieldType)
                {
                    case SQLITE_INTEGER:
                        field->setInt64(locked([&] { return sqlite3_column_int64(statement, columnIndex); }));
                        break;

                    case SQLITE_FLOAT:
                        field->setFloat(locked([&] { return sqlite3_column_double(statement, columnIndex); }));
                        break;

                    case SQLITE_TEXT: {
                        const auto* buffer = locked([&] { return sqlite3_column_text(statement, columnIndex); });
                        const char* text = reinterpret_cast<const char*>(buffer);

                        // Only a computed column (e.g. CURRENT_TIMESTAMP) is a UTC timestamp. A
                        // VARCHAR column holding the same text is user data and must stay a string.
                        const bool isExpression = locked([&] { return sqlite3_column_decltype(statement, columnIndex) == nullptr; });
                        if (isExpression && matchDateTime.matches(text))
                        {
                            const DateTime dateTime(text);
                            field->setDateTime(dateTime - sessionOffset);
                            dataLength = sizeof(int64_t);
                        }
                        else
                        {
                            field->setBuffer(buffer, dataLength, VariantDataType::VAR_BUFFER);
                            dataLength = trimField(bit_cast<char*>(field->get<Buffer>().data()), dataLength);
                        }
                        break;
                    }

                    case SQLITE_BLOB: {
                        const auto* blob = locked([&] { return sqlite3_column_blob(statement, columnIndex); });
                        field->setBuffer(bit_cast<const uint8_t*>(blob), dataLength, VariantDataType::VAR_BUFFER);
                        break;
                    }

                    default:
                        dataLength = 0;
                        break;
                }

                field->dataSize(dataLength);
            }
            else
            {
                setFieldToNull(field, fieldType);
            }
        }
        catch (const Exception& e)
        {
            const auto fieldName = field != nullptr ? field->fieldName() : "";
            throw DatabaseException(
                "Can't read field '" + fieldName + "': " + string(e.what()),
                source_location::current(), query->sql());
        }
    }
}

void SQLite3Connection::setFieldToNull(Field* field, const short sqliteFieldType)
{
    switch (sqliteFieldType)
    {
            using enum VariantDataType;
        case SQLITE_INTEGER:
            field->setInt64(0);
            field->setNull(VAR_INT64);
            break;
        case SQLITE_FLOAT:
            field->setFloat(0);
            field->setNull(VAR_FLOAT);
            break;
        case SQLITE_TEXT:
        case SQLITE_BLOB:
            // An empty buffer, not an empty String. Storing a String and then calling the field a
            // VAR_BUFFER leaves the two disagreeing, and the next row's non-empty value is then
            // assigned to a Buffer that is not there. An empty text column followed by a value in
            // the same column is all it takes, which is what reading a table of accounts where one
            // has no password does.
            field->setBuffer(nullptr, 0, VAR_BUFFER);
            field->setNull(VAR_BUFFER);
            break;
        default:
            field->setString("");
            field->setNull(field->dataType());
            break;
    }
}

void SQLite3Connection::executeBatchSQL(const Strings& batchSQL, Strings* errors)
{
    static const RegularExpression matchStatementEnd("(;\\s*)$");
    static const RegularExpression matchCommentRow("^\\s*--");
    static const RegularExpression matchTrigger("^CREATE(\\s+TEMP(ORARY)?)?\\s+TRIGGER", "i");
    static const RegularExpression matchBlockEnd("^\\s*END\\s*;?\\s*$", "i");

    Strings statements;
    string  statement;
    bool    inTrigger = false;
    for (auto row: batchSQL)
    {
        row = trim(row);
        if (row.empty() || matchCommentRow.matches(row))
        {
            continue;
        }

        if (matchTrigger.matches(row))
        {
            inTrigger = true;
        }

        if (!inTrigger && matchStatementEnd.matches(row))
        {
            row = matchStatementEnd.s(row, "");
            statement += row;
            statements.push_back(trim(statement));
            statement.clear();
            continue;
        }

        statement += row + "\n";

        if (inTrigger && matchBlockEnd.matches(row))
        {
            inTrigger = false;
            statements.push_back(trim(statement));
            statement.clear();
        }
    }

    if (!trim(statement).empty())
    {
        statements.push_back(trim(statement));
    }

    // Execute directly: a batch statement is one-shot and must not require a shared_ptr to this
    // connection or go through prepare/step/finalize for every line.
    const scoped_lock lock(m_mutex);
    if (!active())
    {
        open();
    }

    for (const auto& sql: statements)
    {
        char* zErrMsg = nullptr;
        if (sqlite3_exec(m_connect.get(), sql.c_str(), nullptr, nullptr, &zErrMsg) != SQLITE_OK)
        {
            const String error = zErrMsg != nullptr ? String(zErrMsg) : String(sqlite3_errmsg(m_connect.get()));
            sqlite3_free(zErrMsg);
            stringstream message;
            message << error << ". Query: " << sql;
            if (errors != nullptr)
            {
                errors->push_back(message.str());
            }
            else
            {
                throw DatabaseException(message.str());
            }
        }
    }
}

void SQLite3Connection::objectList(const DatabaseObjectType objectType, Strings& objects)
{
    String objectTypeName;
    switch (objectType)
    {
        case DatabaseObjectType::TABLES:
            objectTypeName = "table";
            break;

        case DatabaseObjectType::VIEWS:
            objectTypeName = "view";
            break;

        default:
            objects.clear();
            return; // no information about objects of other types
    }
    objects.clear();

    const scoped_lock lock(m_mutex);
    if (!active())
    {
        open();
    }

    const String sql("SELECT name FROM sqlite_master WHERE type = ?");
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(m_connect.get(), sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
    {
        throw DatabaseException(String("Can't prepare object list: ") + sqlite3_errmsg(m_connect.get()));
    }
    sqlite3_bind_text(stmt, 1, objectTypeName.c_str(), static_cast<int>(objectTypeName.size()), SQLITE_TRANSIENT);

    int rc = SQLITE_OK;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW)
    {
        if (const auto* name = sqlite3_column_text(stmt, 0); name != nullptr)
        {
            objects.emplace_back(reinterpret_cast<const char*>(name));
        }
    }
    sqlite3_finalize(stmt);

    if (rc != SQLITE_DONE)
    {
        throw DatabaseException(String("Can't read object list: ") + sqlite3_errmsg(m_connect.get()));
    }
}

String SQLite3Connection::driverDescription() const
{
    return "SQLite3 " SQLITE_VERSION;
}

void SQLite3Connection::queryColAttributes(Query*, int16_t, int16_t, int32_t&)
{
    notImplemented("queryColAttributes");
}

void SQLite3Connection::queryColAttributes(Query*, int16_t, int16_t, char*, int)
{
    notImplemented("queryColAttributes");
}

SynchronizedMap<SQLite3Connection*, shared_ptr<SQLite3Connection>> SQLite3Connection::s_sqlite3Connections;

[[maybe_unused]] void* sqlite3CreateConnection(const char* connectionString, const size_t connectionTimeoutSeconds)
{
    const auto connection = make_shared<SQLite3Connection>(connectionString, chrono::seconds(connectionTimeoutSeconds));
    SQLite3Connection::s_sqlite3Connections.insert(connection.get(), connection);
    return connection.get();
}

[[maybe_unused]] void sqlite3DestroyConnection(void* connection)
{
    SQLite3Connection::s_sqlite3Connections.erase(bit_cast<SQLite3Connection*>(connection));
}

bool SQLite3Connection::supportsReturning() const
{
    // 3.35.0, in SQLite's own encoding: major*1000000 + minor*1000 + patch.
    constexpr int firstVersionWithReturning = 3035000;
    return sqlite3_libversion_number() >= firstVersionWithReturning;
}

int64_t SQLite3Connection::lastInsertId() const
{
    auto* handle = connection();
    if (handle == nullptr)
    {
        throw Exception("Database connection is not open");
    }
    return sqlite3_last_insert_rowid(handle);
}

#endif
