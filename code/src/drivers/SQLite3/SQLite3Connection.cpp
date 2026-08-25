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

void SQLite3Connection::closeAndClean()
{
    try
    {
        if (getInTransaction() && SQLite3Connection::active())
        {
            rollbackTransaction();
        }
        close();
    }
    catch (const Exception& e)
    {
        CERR(e.what());
    }
}

String SQLite3Connection::nativeConnectionString() const
{
    if (connectionString().portNumber() > 0)
    {
        throw DatabaseException("Invalid connection string");
    }

    return "/" + connectionString().databaseName() + "/" + connectionString().schema();
}

chrono::minutes SQLite3Connection::getSessionTimezoneOffset()
{
    const auto self = shared_from_this();
    if (self == nullptr)
    {
        throw DatabaseException("PoolDatabaseConnection is not created as shared_ptr");
    }

    m_sessionTimezoneOffset = chrono::minutes(0);
    Query query(self, "SELECT CURRENT_TIMESTAMP");
    const auto  timestamp = query.scalar().asDateTime();
    const auto  sessionTimezoneOffset = chrono::duration_cast<chrono::minutes>(timestamp - DateTime::Now());
    return sessionTimezoneOffset;
}

void SQLite3Connection::_openDatabase(const String& newConnectionString)
{
    if (!active())
    {
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

        m_connect = shared_ptr<sqlite3>(connect,
                                        [this](const sqlite3*)
                                        {
                                            closeAndClean();
                                        });

        const Strings pragmas {
            "pragma journal_mode = WAL",
            "pragma synchronous = off",
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
    disconnectAllQueries();
    switch (sqlite3_close(m_connect.get()))
    {
        case SQLITE_OK:
            break;
        case SQLITE_BUSY:
            throw DatabaseException("Failed to close SQLite3 connection: The database is busy.");
        default:
            throw DatabaseException("Failed to close SQLite3 connection");
    }
    m_connect = nullptr;
}

DBHandle SQLite3Connection::handle() const
{
    return bit_cast<DBHandle>(m_connect.get());
}

bool SQLite3Connection::active() const
{
    return m_connect != nullptr;
}

void SQLite3Connection::driverBeginTransaction()
{
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
    return sqlite3_errmsg(m_connect.get());
}

// Doesn't allocate a statement, but makes sure the previously allocated stmt is released.
void SQLite3Connection::queryAllocStmt(Query* query)
{
    const scoped_lock lock(m_mutex);

    if (auto* stmt = bit_cast<SQLHSTMT>(query->statement());
        stmt != nullptr)
    {
        sqlite3_finalize(stmt);
    }

    querySetStmt(query, nullptr);
}

void SQLite3Connection::queryFreeStmt(Query* query)
{
    const scoped_lock lock(m_mutex);

    querySetStmt(query, nullptr);
    querySetPrepared(query, false);
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
                                            nullptr);
                    break;

                case VAR_BUFFER:
                    res = sqlite3_bind_blob(stmt, parameterBindNumber, parameter->getString(), static_cast<int>(parameter->dataSize()),
                                            nullptr);
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
                                      nullptr);
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

    const scoped_lock lock(m_mutex);

    switch (sqlite3_step(statement))
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

    SQLite3Field*  field = nullptr;
    const uint8_t* buffer = nullptr;
    const char*    text = nullptr;

    for (uint32_t column = 0; column < fieldCount; ++column)
    {
        try
        {
            field = bit_cast<SQLite3Field*>(&(*query)[column]);

            auto fieldType = static_cast<short>(field->fieldType());
            if (fieldType == 0 || fieldType == SQLITE_NULL)
            {
                fieldType = static_cast<short>(sqlite3_column_type(statement, static_cast<int>(column)));
                field->setFieldType(fieldType, 0, 0);
            }

            if (auto dataLength = static_cast<uint32_t>(sqlite3_column_bytes(statement, static_cast<int>(column)));
                dataLength != 0)
            {
                switch (fieldType)
                {
                    case SQLITE_INTEGER:
                        field->setInt64(sqlite3_column_int64(statement, static_cast<int>(column)));
                        break;

                    case SQLITE_FLOAT:
                        field->setFloat(sqlite3_column_double(statement, static_cast<int>(column)));
                        break;

                    case SQLITE_TEXT:
                        buffer = sqlite3_column_text(statement, static_cast<int>(column));
                        text = reinterpret_cast<const char*>(buffer);
                        if (matchDateTime.matches(text))
                        {
                            DateTime dateTime(text);
                            field->setDateTime(dateTime - m_sessionTimezoneOffset);
                            dataLength = sizeof(int64_t);
                        }
                        else
                        {
                            field->setBuffer(buffer, dataLength, VariantDataType::VAR_BUFFER);
                            dataLength = trimField(bit_cast<char*>(field->get<Buffer>().data()), dataLength);
                        }
                        break;

                    case SQLITE_BLOB:
                        field->setBuffer(bit_cast<const uint8_t*>(sqlite3_column_blob(statement, static_cast<int>(column))), dataLength,
                                         VariantDataType::VAR_BUFFER);
                        break;

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
            field->setString("");
            field->setNull(VAR_BUFFER);
            break;
        default:
            field->setString("");
            field->setNull(VAR_NONE);
            break;
    }
}

void SQLite3Connection::executeBatchSQL(const Strings& batchSQL, Strings* errors)
{
    static const RegularExpression matchStatementEnd("(;\\s*)$");
    static const RegularExpression matchCommentRow("^\\s*--");

    Strings statements;
    string  statement;
    for (String row: batchSQL)
    {
        row = trim(row);
        if (row.empty() || matchCommentRow.matches(row))
        {
            continue;
        }

        row = trim(row);
        if (row.empty() || row.starts_with("--"))
        {
            continue;
        }

        if (matchStatementEnd.matches(row))
        {
            row = matchStatementEnd.s(row, "");
            statement += row;
            statements.push_back(trim(statement));
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
        try
        {
            Query query(self, stmt, false);
            query.exec();
        }
        catch (const Exception& e)
        {
            stringstream error;
            error << e.what() << ". Query: " << stmt;
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

void SQLite3Connection::objectList(const DatabaseObjectType objectType, Strings& objects)
{
    string objectTypeName;
    objects.clear();

    switch (objectType)
    {
        case DatabaseObjectType::TABLES:
            objectTypeName = "table";
            break;

        case DatabaseObjectType::VIEWS:
            objectTypeName = "view";
            break;

        default:
            return; // no information about objects of other types
    }

    const auto self = shared_from_this();
    if (self == nullptr)
    {
        throw DatabaseException("PoolDatabaseConnection is not created as shared_ptr");
    }

    Query query(self, "SELECT name FROM sqlite_master WHERE type='" + objectTypeName + "'");
    query.open();

    while (!query.eof())
    {
        objects.push_back(query[static_cast<uint32_t>(0)].asString());
        query.next();
    }

    query.close();
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

#endif
