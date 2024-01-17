/*
╔══════════════════════════════════════════════════════════════════════════════╗
║                       SIMPLY POWERFUL TOOLKIT (SPTK)                         ║
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

#include <sptk5/sptk.h>

#ifdef HAVE_SQLITE3

#include <sptk5/cutils>
#include <sptk5/db/DatabaseField.h>
#include <sptk5/db/Query.h>
#include <sptk5/db/SQLite3Connection.h>

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

SQLite3Connection::SQLite3Connection(const String& connectionString, chrono::seconds connectTimeout)
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
        CERR(e.what() << endl);
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
        if (sqlite3_open(nativeConnectionString().c_str(), &connect) != 0)
        {
            const String error = sqlite3_errmsg(connect);
            sqlite3_close(connect);
            m_connect.reset();
            throw DatabaseException(error);
        }

        m_connect = shared_ptr<sqlite3>(connect,
                                        [this](const sqlite3*) {
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
                throw DatabaseException(zErrMsg);
            }
        }
    }
}

void SQLite3Connection::closeDatabase()
{
    disconnectAllQueries();
    sqlite3_close(m_connect.get());
    m_connect = nullptr;
}

DBHandle SQLite3Connection::handle() const
{
    return (DBHandle) m_connect.get();
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
        throw DatabaseException(zErrMsg);
    }

    setInTransaction(true);
}

void SQLite3Connection::driverEndTransaction(bool commit)
{
    if (!getInTransaction())
    {
        throw DatabaseException("Transaction isn't started.");
    }

    const char* action = commit ? "COMMIT" : "ROLLBACK";

    if (char* zErrMsg = nullptr;
        sqlite3_exec(m_connect.get(), action, nullptr, nullptr, &zErrMsg) != SQLITE_OK)
    {
        throw DatabaseException(zErrMsg);
    }

    setInTransaction(false);
}

//-----------------------------------------------------------------------------------------------

String SQLite3Connection::queryError(const Query*) const
{
    return sqlite3_errmsg(m_connect.get());
}

// Doesn't actually allocate stmt, but makes sure
// the previously allocated stmt is released
void SQLite3Connection::queryAllocStmt(Query* query)
{
    const scoped_lock lock(m_mutex);

    if (auto* stmt = (SQLHSTMT) query->statement();
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
    queryFreeStmt(query);
}

void SQLite3Connection::queryPrepare(Query* query)
{
    const scoped_lock lock(m_mutex);

    SQLHSTMT hStmt = nullptr;

    if (const char* pzTail = nullptr;
        sqlite3_prepare_v2(m_connect.get(), query->sql().c_str(), int(query->sql().length()), &hStmt, &pzTail) !=
        SQLITE_OK)
    {
        const char* errorMsg = sqlite3_errmsg(m_connect.get());
        throw DatabaseException(errorMsg, source_location::current(), query->sql());
    }

    auto statement = shared_ptr<uint8_t>((StmtHandle) hStmt,
                                         [](StmtHandle ptr) {
                                             auto* stmt = (SQLHSTMT) ptr;
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

    auto* stmt = (SQLHSTMT) query->statement();

    return sqlite3_column_count(stmt);
}

void SQLite3Connection::queryBindParameters(Query* query)
{
    const scoped_lock lock(m_mutex);

    auto* stmt = (SQLHSTMT) query->statement();
    sqlite3_reset(stmt);
    sqlite3_clear_bindings(stmt);

    for (uint32_t i = 0; i < query->paramCount(); ++i)
    {
        bindParameter(query, i);
    }
}

void SQLite3Connection::bindParameter(const Query* query, uint32_t paramNumber) const
{
    auto* stmt = (SQLHSTMT) query->statement();
    QueryParameter* param = &query->param(paramNumber);
    const VariantDataType ptype = param->dataType();

    for (unsigned j = 0; j < param->bindCount(); ++j)
    {
        int res;
        auto paramBindNumber = short(param->bindIndex(j) + 1);

        if (param->isNull())
        {
            res = sqlite3_bind_null(stmt, paramBindNumber);
        }
        else
        {
            switch (ptype)
            {
                using enum sptk::VariantDataType;
                case VAR_BOOL:
                    res = sqlite3_bind_int(stmt, paramBindNumber, param->get<bool>());
                    break;

                case VAR_INT:
                    res = sqlite3_bind_int(stmt, paramBindNumber, param->get<int>());
                    break;

                case VAR_INT64:
                    res = sqlite3_bind_int64(stmt, paramBindNumber, param->get<int64_t>());
                    break;

                case VAR_FLOAT:
                    res = sqlite3_bind_double(stmt, paramBindNumber, param->get<double>());
                    break;

                case VAR_DATE_TIME:
                    res = transformDateTimeParameter(stmt, param, paramBindNumber);
                    break;

                case VAR_STRING:
                case VAR_TEXT:
                    res = sqlite3_bind_text(stmt, paramBindNumber, param->getString(), int(param->dataSize()),
                                            nullptr);
                    break;

                case VAR_BUFFER:
                    res = sqlite3_bind_blob(stmt, paramBindNumber, param->getString(), int(param->dataSize()),
                                            nullptr);
                    break;

                default:
                    throw DatabaseException(
                        vformat("Unsupported parameter type ({}) for parameter '{}'",
                                make_format_args((int) param->dataType(), param->name().c_str())));
            }
        }

        if (res != SQLITE_OK)
        {
            const String error = sqlite3_errmsg(m_connect.get());
            sqlite3_finalize(stmt);
            throw DatabaseException(
                vformat("{}, in binding parameter '{}'", make_format_args(error.c_str(), param->name().c_str())),
                source_location::current(), query->sql());
        }
    }
}

int SQLite3Connection::transformDateTimeParameter(sqlite3_stmt* stmt, QueryParameter* param, short paramBindNumber)
{
    const auto dt = param->get<DateTime>();
    param->setString(dt.isoDateTimeString());
    auto rc = sqlite3_bind_text(stmt, paramBindNumber, param->getString(), int(param->dataSize()),
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

    auto count = (short) queryColCount(query);

    query->fields().clear();

    auto* stmt = (SQLHSTMT) query->statement();

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
uint32_t trimField(char* str, uint32_t length)
{
    if (length == 0)
    {
        return 0;
    }

    char* p = str + length - 1;
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
    return uint32_t(p - str);
}
} // namespace

void SQLite3Connection::queryFetch(Query* query)
{
    if (!query->active())
    {
        throw DatabaseException("Dataset isn't open", source_location::current(), query->sql());
    }

    auto* statement = (SQLHSTMT) query->statement();

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

    auto fieldCount = query->fieldCount();
    if (fieldCount == 0)
    {
        return;
    }

    SQLite3Field* field = nullptr;

    for (uint32_t column = 0; column < fieldCount; ++column)
    {
        try
        {
            field = bit_cast<SQLite3Field*>(&(*query)[column]);

            auto fieldType = (short) field->fieldType();
            if (fieldType == 0 || fieldType == SQLITE_NULL)
            {
                fieldType = (short) sqlite3_column_type(statement, int(column));
                field->setFieldType(fieldType, 0, 0);
            }

            auto dataLength = (uint32_t) sqlite3_column_bytes(statement, int(column));

            if (dataLength != 0)
            {
                switch (fieldType)
                {
                    case SQLITE_INTEGER:
                        field->setInt64(sqlite3_column_int64(statement, int(column)));
                        break;

                    case SQLITE_FLOAT:
                        field->setFloat(sqlite3_column_double(statement, int(column)));
                        break;

                    case SQLITE_TEXT:
                        field->setBuffer(sqlite3_column_text(statement, int(column)), dataLength,
                                         VariantDataType::VAR_BUFFER);
                        dataLength = trimField(bit_cast<char*>(field->get<Buffer>().data()), dataLength);
                        break;

                    case SQLITE_BLOB:
                        field->setBuffer(bit_cast<const uint8_t*>(sqlite3_column_blob(statement, int(column))), dataLength,
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
                field->setString("");
                field->setNull(VariantDataType::VAR_NONE);
            }
        }
        catch (const Exception& e)
        {
            const auto fieldName = field != nullptr ? field->fieldName() : "";
            throw DatabaseException(
                vformat("Can't read field '{}': {}", make_format_args(fieldName.c_str(), e.what())),
                source_location::current(), query->sql());
        }
    }
}

void SQLite3Connection::executeBatchSQL(const sptk::Strings& sqlBatch, Strings* errors)
{
    static const RegularExpression matchStatementEnd("(;\\s*)$");
    static const RegularExpression matchCommentRow("^\\s*--");

    Strings statements;
    string statement;
    for (String row: sqlBatch)
    {
        row = trim(row);
        if (row.empty() || matchCommentRow.matches(row))
        {
            continue;
        }

        row = trim(row);
        if (row.empty() || row.startsWith("--"))
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

    for (const auto& stmt: statements)
    {
        try
        {
            Query query(this, stmt, false);
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

void SQLite3Connection::objectList(DatabaseObjectType objectType, Strings& objects)
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

    Query query(this, "SELECT name FROM sqlite_master WHERE type='" + objectTypeName + "'");
    query.open();

    while (!query.eof())
    {
        objects.push_back(query[uint32_t(0)].asString());
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

map<SQLite3Connection*, shared_ptr<SQLite3Connection>> SQLite3Connection::s_sqlite3Connections;

[[maybe_unused]] void* sqlite3_create_connection(const char* connectionString, size_t connectionTimeoutSeconds)
{
    auto connection = make_shared<SQLite3Connection>(connectionString, chrono::seconds(connectionTimeoutSeconds));
    SQLite3Connection::s_sqlite3Connections[connection.get()] = connection;
    return connection.get();
}

[[maybe_unused]] void sqlite3_destroy_connection(void* connection)
{
    SQLite3Connection::s_sqlite3Connections.erase(bit_cast<SQLite3Connection*>(connection));
}

#endif
